#include <iostream>
#include <fstream>
#include <vector>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/foreach.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/test/unit_test.hpp>
#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_writer_template.h"
#include "json/json_spirit_utils.h"
#include "script_error.h"


#include "main.h"
#include "wallet.h"

using namespace std;
using namespace json_spirit;
using namespace boost::algorithm;

extern uint256 SignatureHash(CScript scriptCode, const CTransaction& txTo, unsigned int nIn, int nHashType);
extern bool VerifyScript(const CScript& scriptSig, const CScript& scriptPubKey, const CTransaction& txTo, unsigned int nIn, bool fValidatePayToScriptHash, int nHashType, ScriptError* serror);

static const unsigned int flags = SCRIPT_VERIFY_P2SH | SCRIPT_VERIFY_STRICTENC;

CScript
ParseScript(string s)
{
    CScript result;

    static map<string, opcodetype> mapOpNames;

    if (mapOpNames.size() == 0)
    {
        for (int op = OP_NOP; op <= OP_NOP10; op++)
        {
            const char* name = GetOpName((opcodetype)op);
            if (strcmp(name, "OP_UNKNOWN") == 0)
                continue;
            string strName(name);
            mapOpNames[strName] = (opcodetype)op;
            // Convenience: OP_ADD and just ADD are both recognized:
            replace_first(strName, "OP_", "");
            mapOpNames[strName] = (opcodetype)op;
        }
    }

    vector<string> words;
    split(words, s, is_any_of(" \t\n"), token_compress_on);

    BOOST_FOREACH(string w, words)
    {
        if (all(w, is_digit()) ||
            (starts_with(w, "-") && all(string(w.begin()+1, w.end()), is_digit())))
        {
            // Number
            int64 n = atoi64(w);
            result << n;
        }
        else if (starts_with(w, "0x") && IsHex(string(w.begin()+2, w.end())))
        {
            // Raw hex data, inserted NOT pushed onto stack:
            std::vector<unsigned char> raw = ParseHex(string(w.begin()+2, w.end()));
            result.insert(result.end(), raw.begin(), raw.end());
        }
        else if (w.size() >= 2 && starts_with(w, "'") && ends_with(w, "'"))
        {
            // Single-quoted string, pushed as data. NOTE: this is poor-man's
            // parsing, spaces/tabs/newlines in single-quoted strings won't work.
            std::vector<unsigned char> value(w.begin()+1, w.end()-1);
            result << value;
        }
        else if (mapOpNames.count(w))
        {
            // opcode, e.g. OP_ADD or OP_1:
            result << mapOpNames[w];
        }
        else
        {
            BOOST_ERROR("Parse error: " << s);
            return CScript();
        }
    }

    return result;
}

Array
read_json(const std::string& filename)
{
    namespace fs = boost::filesystem;
    fs::path testFile = fs::current_path() / "test" / "data" / filename;

#ifdef TEST_DATA_DIR
    if (!fs::exists(testFile))
    {
        testFile = fs::path(BOOST_PP_STRINGIZE(TEST_DATA_DIR)) / filename;
    }
#endif

    ifstream ifs(testFile.string().c_str(), ifstream::in);
    Value v;
    if (!read_stream(ifs, v))
    {
        if (ifs.fail())
            BOOST_ERROR("Cound not find/open " << filename);
        else
            BOOST_ERROR("JSON syntax error in " << filename);
        return Array();
    }
    if (v.type() != array_type)
    {
        BOOST_ERROR(filename << " does not contain a json array");
        return Array();
    }

    return v.get_array();
}

BOOST_AUTO_TEST_SUITE(script_tests)

BOOST_AUTO_TEST_CASE(script_valid)
{
    // Read tests from test/data/script_valid.json
    // Format is an array of arrays
    // Inner arrays are [ "scriptSig", "scriptPubKey" ]
    // ... where scriptSig and scriptPubKey are stringified
    // scripts.
    Array tests = read_json("script_valid.json");
    unsigned int xflags = SCRIPT_VERIFY_P2SH | SCRIPT_VERIFY_STRICTENC | SCRIPT_VERIFY_CHECKLOCKTIMEVERIFY;

    ScriptError err;

    BOOST_FOREACH(Value& tv, tests)
    {
        Array test = tv.get_array();
        string strTest = write_string(tv, false);
        if (test.size() < 2) // Allow size > 2; extra stuff ignored (useful for comments)
        {
            BOOST_ERROR("Bad test: " << strTest);
            continue;
        }
        string scriptSigString = test[0].get_str();
        CScript scriptSig = ParseScript(scriptSigString);
        string scriptPubKeyString = test[1].get_str();
        CScript scriptPubKey = ParseScript(scriptPubKeyString);

        CTransaction tx;
        BOOST_CHECK_MESSAGE(VerifyScript(scriptSig, scriptPubKey, tx, 0, true, xflags, SIGHASH_NONE, &err), strTest);
        if (err > 0)
            BOOST_TEST_MESSAGE("err " << err << " -- " << ScriptErrorString(err));
    }
}

BOOST_AUTO_TEST_CASE(script_invalid)
{
    // Scripts that should evaluate as invalid
    Array tests = read_json("script_invalid.json");
    unsigned int xflags = SCRIPT_VERIFY_P2SH | SCRIPT_VERIFY_STRICTENC | SCRIPT_VERIFY_CHECKLOCKTIMEVERIFY;

    ScriptError err;

    BOOST_FOREACH(Value& tv, tests)
    {
        Array test = tv.get_array();
        string strTest = write_string(tv, false);
        if (test.size() < 2) // Allow size > 2; extra stuff ignored (useful for comments)
        {
            BOOST_ERROR("Bad test: " << strTest);
            continue;
        }
        string scriptSigString = test[0].get_str();
        CScript scriptSig = ParseScript(scriptSigString);
        string scriptPubKeyString = test[1].get_str();
        CScript scriptPubKey = ParseScript(scriptPubKeyString);

        CTransaction tx;
        BOOST_CHECK_MESSAGE(!VerifyScript(scriptSig, scriptPubKey, tx, 0, true, xflags, SIGHASH_NONE, &err), strTest);
    }
}

BOOST_AUTO_TEST_CASE(script_PushData)
{
    // Check that PUSHDATA1, PUSHDATA2, and PUSHDATA4 create the same value on
    // the stack as the 1-75 opcodes do.
    static const unsigned char direct[] = { 1, 0x5a };
    static const unsigned char pushdata1[] = { OP_PUSHDATA1, 1, 0x5a };
    static const unsigned char pushdata2[] = { OP_PUSHDATA2, 1, 0, 0x5a };
    static const unsigned char pushdata4[] = { OP_PUSHDATA4, 1, 0, 0, 0, 0x5a };
    unsigned int xflags = SCRIPT_VERIFY_P2SH | SCRIPT_VERIFY_STRICTENC | SCRIPT_VERIFY_CHECKLOCKTIMEVERIFY;

    ScriptError err;

    vector<vector<unsigned char> > directStack;
    BOOST_CHECK(EvalScript(directStack, CScript(&direct[0], &direct[sizeof(direct)]), CTransaction(), 0, true, 0, &err));

    vector<vector<unsigned char> > pushdata1Stack;
    BOOST_CHECK(EvalScript(pushdata1Stack, CScript(&pushdata1[0], &pushdata1[sizeof(pushdata1)]), CTransaction(), 0, true, 0, &err));
    BOOST_CHECK(pushdata1Stack == directStack);

    vector<vector<unsigned char> > pushdata2Stack;
    BOOST_CHECK(EvalScript(pushdata2Stack, CScript(&pushdata2[0], &pushdata2[sizeof(pushdata2)]), CTransaction(), 0, true, 0, &err));
    BOOST_CHECK(pushdata2Stack == directStack);

    vector<vector<unsigned char> > pushdata4Stack;
    BOOST_CHECK(EvalScript(pushdata4Stack, CScript(&pushdata4[0], &pushdata4[sizeof(pushdata4)]), CTransaction(), 0, true, 0, &err));
    BOOST_CHECK(pushdata4Stack == directStack);
}

CScript
sign_multisig(CScript scriptPubKey, std::vector<CKey> keys, CTransaction transaction)
{
    uint256 hash = SignatureHash(scriptPubKey, transaction, 0, SIGHASH_ALL);

    CScript result;
    //
    // NOTE: CHECKMULTISIG has an unfortunate bug; it requires
    // one extra item on the stack, before the signatures.
    // Putting OP_0 on the stack is the workaround;
    // fixing the bug would mean splitting the block chain (old
    // clients would not accept new CHECKMULTISIG transactions,
    // and vice-versa)
    //
    result << OP_0;
    BOOST_FOREACH(CKey key, keys)
    {
        vector<unsigned char> vchSig;
        BOOST_CHECK(key.Sign(hash, vchSig));
        vchSig.push_back((unsigned char)SIGHASH_ALL);
        result << vchSig;
    }
    return result;
}
CScript
sign_multisig(CScript scriptPubKey, CKey key, CTransaction transaction)
{
    std::vector<CKey> keys;
    keys.push_back(key);
    return sign_multisig(scriptPubKey, keys, transaction);
}

BOOST_AUTO_TEST_CASE(script_CHECKMULTISIG12)
{
    unsigned int xflags = SCRIPT_VERIFY_P2SH | SCRIPT_VERIFY_STRICTENC | SCRIPT_VERIFY_CHECKLOCKTIMEVERIFY;

    ScriptError err;

    CKey key1, key2, key3;
    key1.MakeNewKey(true);
    key2.MakeNewKey(false);
    key3.MakeNewKey(true);

    CScript scriptPubKey12;
    scriptPubKey12 << OP_1 << key1.GetPubKey() << key2.GetPubKey() << OP_2 << OP_CHECKMULTISIG;

    CTransaction txFrom12;
    txFrom12.vout.resize(1);
    txFrom12.vout[0].scriptPubKey = scriptPubKey12;

    CTransaction txTo12;
    txTo12.vin.resize(1);
    txTo12.vout.resize(1);
    txTo12.vin[0].prevout.n = 0;
    txTo12.vin[0].prevout.hash = txFrom12.GetHash();
    txTo12.vout[0].nValue = 1;

    CScript goodsig1 = sign_multisig(scriptPubKey12, key1, txTo12);
    BOOST_CHECK(VerifyScript(goodsig1, scriptPubKey12, txTo12, 0, true, xflags, 0, &err));
    txTo12.vout[0].nValue = 2;
    BOOST_CHECK(!VerifyScript(goodsig1, scriptPubKey12, txTo12, 0, true, xflags, 0, &err));

    CScript goodsig2 = sign_multisig(scriptPubKey12, key2, txTo12);
    BOOST_CHECK(VerifyScript(goodsig2, scriptPubKey12, txTo12, 0, true, xflags, 0, &err));

    CScript badsig1 = sign_multisig(scriptPubKey12, key3, txTo12);
    BOOST_CHECK(!VerifyScript(badsig1, scriptPubKey12, txTo12, 0, true, xflags, 0, &err));
}

BOOST_AUTO_TEST_CASE(script_CHECKMULTISIG23)
{
    unsigned int xflags = SCRIPT_VERIFY_P2SH | SCRIPT_VERIFY_STRICTENC | SCRIPT_VERIFY_CHECKLOCKTIMEVERIFY;

    ScriptError err;

    CKey key1, key2, key3, key4;
    key1.MakeNewKey(true);
    key2.MakeNewKey(false);
    key3.MakeNewKey(true);
    key4.MakeNewKey(false);

    CScript scriptPubKey23;
    scriptPubKey23 << OP_2 << key1.GetPubKey() << key2.GetPubKey() << key3.GetPubKey() << OP_3 << OP_CHECKMULTISIG;

    CTransaction txFrom23;
    txFrom23.vout.resize(1);
    txFrom23.vout[0].scriptPubKey = scriptPubKey23;

    CTransaction txTo23;
    txTo23.vin.resize(1);
    txTo23.vout.resize(1);
    txTo23.vin[0].prevout.n = 0;
    txTo23.vin[0].prevout.hash = txFrom23.GetHash();
    txTo23.vout[0].nValue = 1;

    std::vector<CKey> keys;
    keys.push_back(key1); keys.push_back(key2);
    CScript goodsig1 = sign_multisig(scriptPubKey23, keys, txTo23);
    BOOST_CHECK(VerifyScript(goodsig1, scriptPubKey23, txTo23, 0, true, xflags, 0, &err));

    keys.clear();
    keys.push_back(key1); keys.push_back(key3);
    CScript goodsig2 = sign_multisig(scriptPubKey23, keys, txTo23);
    BOOST_CHECK(VerifyScript(goodsig2, scriptPubKey23, txTo23, 0, true, xflags, 0, &err));

    keys.clear();
    keys.push_back(key2); keys.push_back(key3);
    CScript goodsig3 = sign_multisig(scriptPubKey23, keys, txTo23);
    BOOST_CHECK(VerifyScript(goodsig3, scriptPubKey23, txTo23, 0, true, xflags, 0, &err));

    keys.clear();
    keys.push_back(key2); keys.push_back(key2); // Can't re-use sig
    CScript badsig1 = sign_multisig(scriptPubKey23, keys, txTo23);
    BOOST_CHECK(!VerifyScript(badsig1, scriptPubKey23, txTo23, 0, true, xflags, 0, &err));

    keys.clear();
    keys.push_back(key2); keys.push_back(key1); // sigs must be in correct order
    CScript badsig2 = sign_multisig(scriptPubKey23, keys, txTo23);
    BOOST_CHECK(!VerifyScript(badsig2, scriptPubKey23, txTo23, 0, true, xflags, 0, &err));

    keys.clear();
    keys.push_back(key3); keys.push_back(key2); // sigs must be in correct order
    CScript badsig3 = sign_multisig(scriptPubKey23, keys, txTo23);
    BOOST_CHECK(!VerifyScript(badsig3, scriptPubKey23, txTo23, 0, true, xflags, 0, &err));

    keys.clear();
    keys.push_back(key4); keys.push_back(key2); // sigs must match pubkeys
    CScript badsig4 = sign_multisig(scriptPubKey23, keys, txTo23);
    BOOST_CHECK(!VerifyScript(badsig4, scriptPubKey23, txTo23, 0, true, xflags, 0, &err));

    keys.clear();
    keys.push_back(key1); keys.push_back(key4); // sigs must match pubkeys
    CScript badsig5 = sign_multisig(scriptPubKey23, keys, txTo23);
    BOOST_CHECK(!VerifyScript(badsig5, scriptPubKey23, txTo23, 0, true, xflags, 0, &err));

    keys.clear(); // Must have signatures
    CScript badsig6 = sign_multisig(scriptPubKey23, keys, txTo23);
    BOOST_CHECK(!VerifyScript(badsig6, scriptPubKey23, txTo23, 0, true, xflags, 0, &err));
}    

BOOST_AUTO_TEST_CASE(script_combineSigs)
{
    // Test the CombineSignatures function
    CBasicKeyStore keystore;
    vector<CKey> keys;
    for (int i = 0; i < 3; i++)
    {
        CKey key;
        key.MakeNewKey(i%2 == 1);
        keys.push_back(key);
        keystore.AddKey(key);
    }

    CTransaction txFrom;
    txFrom.vout.resize(1);
    txFrom.vout[0].scriptPubKey.SetDestination(keys[0].GetPubKey().GetID());
    CScript& scriptPubKey = txFrom.vout[0].scriptPubKey;
    CTransaction txTo;
    txTo.vin.resize(1);
    txTo.vout.resize(1);
    txTo.vin[0].prevout.n = 0;
    txTo.vin[0].prevout.hash = txFrom.GetHash();
    CScript& scriptSig = txTo.vin[0].scriptSig;
    txTo.vout[0].nValue = 1;

    CScript empty;
    CScript combined = CombineSignatures(scriptPubKey, txTo, 0, empty, empty);
    BOOST_CHECK(combined.empty());

    // Single signature case:
    SignSignature(keystore, txFrom, txTo, 0); // changes scriptSig
    combined = CombineSignatures(scriptPubKey, txTo, 0, scriptSig, empty);
    BOOST_CHECK(combined == scriptSig);
    combined = CombineSignatures(scriptPubKey, txTo, 0, empty, scriptSig);
    BOOST_CHECK(combined == scriptSig);
    CScript scriptSigCopy = scriptSig;
    // Signing again will give a different, valid signature:
    SignSignature(keystore, txFrom, txTo, 0);
    combined = CombineSignatures(scriptPubKey, txTo, 0, scriptSigCopy, scriptSig);
    BOOST_CHECK(combined == scriptSigCopy || combined == scriptSig);

    // P2SH, single-signature case:
    CScript pkSingle; pkSingle << keys[0].GetPubKey() << OP_CHECKSIG;
    keystore.AddCScript(pkSingle);
    scriptPubKey.SetDestination(pkSingle.GetID());
    SignSignature(keystore, txFrom, txTo, 0);
    combined = CombineSignatures(scriptPubKey, txTo, 0, scriptSig, empty);
    BOOST_CHECK(combined == scriptSig);
    combined = CombineSignatures(scriptPubKey, txTo, 0, empty, scriptSig);
    BOOST_CHECK(combined == scriptSig);
    scriptSigCopy = scriptSig;
    SignSignature(keystore, txFrom, txTo, 0);
    combined = CombineSignatures(scriptPubKey, txTo, 0, scriptSigCopy, scriptSig);
    BOOST_CHECK(combined == scriptSigCopy || combined == scriptSig);
    // dummy scriptSigCopy with placeholder, should always choose non-placeholder:
    scriptSigCopy = CScript() << OP_0 << static_cast<vector<unsigned char> >(pkSingle);
    combined = CombineSignatures(scriptPubKey, txTo, 0, scriptSigCopy, scriptSig);
    BOOST_CHECK(combined == scriptSig);
    combined = CombineSignatures(scriptPubKey, txTo, 0, scriptSig, scriptSigCopy);
    BOOST_CHECK(combined == scriptSig);

    // Hardest case:  Multisig 2-of-3
    scriptPubKey.SetMultisig(2, keys);
    keystore.AddCScript(scriptPubKey);
    SignSignature(keystore, txFrom, txTo, 0);
    combined = CombineSignatures(scriptPubKey, txTo, 0, scriptSig, empty);
    BOOST_CHECK(combined == scriptSig);
    combined = CombineSignatures(scriptPubKey, txTo, 0, empty, scriptSig);
    BOOST_CHECK(combined == scriptSig);

    // A couple of partially-signed versions:
    vector<unsigned char> sig1;
    uint256 hash1 = SignatureHash(scriptPubKey, txTo, 0, SIGHASH_ALL);
    BOOST_CHECK(keys[0].Sign(hash1, sig1));
    sig1.push_back(SIGHASH_ALL);
    vector<unsigned char> sig2;
    uint256 hash2 = SignatureHash(scriptPubKey, txTo, 0, SIGHASH_NONE);
    BOOST_CHECK(keys[1].Sign(hash2, sig2));
    sig2.push_back(SIGHASH_NONE);
    vector<unsigned char> sig3;
    uint256 hash3 = SignatureHash(scriptPubKey, txTo, 0, SIGHASH_SINGLE);
    BOOST_CHECK(keys[2].Sign(hash3, sig3));
    sig3.push_back(SIGHASH_SINGLE);

    // Not fussy about order (or even existence) of placeholders or signatures:
    CScript partial1a = CScript() << OP_0 << sig1 << OP_0;
    CScript partial1b = CScript() << OP_0 << OP_0 << sig1;
    CScript partial2a = CScript() << OP_0 << sig2;
    CScript partial2b = CScript() << sig2 << OP_0;
    CScript partial3a = CScript() << sig3;
    CScript partial3b = CScript() << OP_0 << OP_0 << sig3;
    CScript partial3c = CScript() << OP_0 << sig3 << OP_0;
    CScript complete12 = CScript() << OP_0 << sig1 << sig2;
    CScript complete13 = CScript() << OP_0 << sig1 << sig3;
    CScript complete23 = CScript() << OP_0 << sig2 << sig3;

    combined = CombineSignatures(scriptPubKey, txTo, 0, partial1a, partial1b);
    BOOST_CHECK(combined == partial1a);
    combined = CombineSignatures(scriptPubKey, txTo, 0, partial1a, partial2a);
    BOOST_CHECK(combined == complete12);
    combined = CombineSignatures(scriptPubKey, txTo, 0, partial2a, partial1a);
    BOOST_CHECK(combined == complete12);
    combined = CombineSignatures(scriptPubKey, txTo, 0, partial1b, partial2b);
    BOOST_CHECK(combined == complete12);
    combined = CombineSignatures(scriptPubKey, txTo, 0, partial3b, partial1b);
    BOOST_CHECK(combined == complete13);
    combined = CombineSignatures(scriptPubKey, txTo, 0, partial2a, partial3a);
    BOOST_CHECK(combined == complete23);
    combined = CombineSignatures(scriptPubKey, txTo, 0, partial3b, partial2b);
    BOOST_CHECK(combined == complete23);
    combined = CombineSignatures(scriptPubKey, txTo, 0, partial3b, partial3a);
    BOOST_CHECK(combined == partial3c);
}

BOOST_AUTO_TEST_CASE(script_CHECKLOCKTIMEVERIFY)
{
    //
    // Simple locktime script
    //

    CScript scriptSimpleLock = CScript() << OP_16 << OP_CHECKLOCKTIMEVERIFY;

    ScriptError err;

    CTransaction tx;
    tx.vin.push_back(CTxIn());
    // Fails since tx.nLockTime < 16
    BOOST_CHECK(!VerifyScript(CScript(), scriptSimpleLock, tx, 0, true, SCRIPT_VERIFY_CHECKLOCKTIMEVERIFY, 0, &err));

    tx.nLockTime = 16;
    // Fails since tx.vin[0].nSequence = CTxIn::SEQUENCE_FINAL by default, which disables nLockTime.
    BOOST_CHECK(!VerifyScript(CScript(), scriptSimpleLock, tx, 0, true, SCRIPT_VERIFY_CHECKLOCKTIMEVERIFY, 0, &err));

    tx.vin[0].nSequence = CTxIn::SEQUENCE_FINAL-1;
    BOOST_CHECK(VerifyScript(CScript(), scriptSimpleLock, tx, 0, true, SCRIPT_VERIFY_CHECKLOCKTIMEVERIFY, 0, &err));

    // Verify larger locktime succeeds
    tx.nLockTime = 17;
    BOOST_CHECK(VerifyScript(CScript(), scriptSimpleLock, tx, 0, true, SCRIPT_VERIFY_CHECKLOCKTIMEVERIFY, 0, &err));

    // Verify smaller locktime fails
    tx.nLockTime = 15;
    BOOST_CHECK(!VerifyScript(CScript(), scriptSimpleLock, tx, 0, true, SCRIPT_VERIFY_CHECKLOCKTIMEVERIFY, 0, &err));

    // Compare timestamps
    int64 nNow = GetTime();
    CScript scriptSimpleTimeLock = CScript() << nNow << OP_CHECKLOCKTIMEVERIFY;
    tx.nLockTime = nNow;
    BOOST_CHECK(VerifyScript(CScript(), scriptSimpleTimeLock, tx, 0, true, SCRIPT_VERIFY_CHECKLOCKTIMEVERIFY, 0, &err));
    tx.nLockTime = nNow+1;
    BOOST_CHECK(VerifyScript(CScript(), scriptSimpleTimeLock, tx, 0, true, SCRIPT_VERIFY_CHECKLOCKTIMEVERIFY, 0, &err));
    tx.nLockTime = nNow-1;
    BOOST_CHECK(!VerifyScript(CScript(), scriptSimpleTimeLock, tx, 0, true, SCRIPT_VERIFY_CHECKLOCKTIMEVERIFY, 0, &err));

    // Verify comparing timestamps with block heights fails
    tx.nLockTime = nNow;
    BOOST_CHECK(!VerifyScript(CScript(), scriptSimpleLock, tx, 0, true, SCRIPT_VERIFY_CHECKLOCKTIMEVERIFY, 0, &err));
    tx.nLockTime = 300000000; // very large block height
    BOOST_CHECK(!VerifyScript(CScript(), scriptSimpleTimeLock, tx, 0, true, SCRIPT_VERIFY_CHECKLOCKTIMEVERIFY, 0, &err));
}

BOOST_AUTO_TEST_SUITE_END()
