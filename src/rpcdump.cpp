// Copyright (c) 2009-2012 Bitcoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "init.h" // for pwalletMain
#include "bitcoinrpc.h"
#include "ui_interface.h"
#include "base58.h"

#include <boost/lexical_cast.hpp>

#define printf OutputDebugStringF

using namespace json_spirit;
using namespace std;

class CTxDump
{
public:
    CBlockIndex *pindex;
    int64 nValue;
    bool fSpent;
    CWalletTx* ptx;
    int nOut;
    CTxDump(CWalletTx* ptx = NULL, int nOut = -1)
    {
        pindex = NULL;
        nValue = 0;
        fSpent = false;
        this->ptx = ptx;
        this->nOut = nOut;
    }
};

Value importprivkey(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 3)
        throw runtime_error(
            "importprivkey <fuguecoinprivkey> [label] [rescan=true]\n"
            "Adds a private key (as returned by dumpprivkey) to your wallet.");

    string strSecret = params[0].get_str();
    string strLabel = "";
    if (params.size() > 1)
        strLabel = params[1].get_str();

    // Whether to perform rescan after import
    bool fRescan = true;
    if (params.size() > 2)
        fRescan = params[2].get_bool();

    CBitcoinSecret vchSecret;
    bool fGood = vchSecret.SetString(strSecret);

    if (!fGood) throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid private key");

    CKey key;
    bool fCompressed;
    CSecret secret = vchSecret.GetSecret(fCompressed);
    key.SetSecret(secret, fCompressed);
    CKeyID vchAddress = key.GetPubKey().GetID();
    {
        LOCK2(cs_main, pwalletMain->cs_wallet);

        pwalletMain->MarkDirty();
        pwalletMain->SetAddressBookName(vchAddress, strLabel);

        if (!pwalletMain->AddKey(key))
            throw JSONRPCError(RPC_WALLET_ERROR, "Error adding key to wallet");
	
        if (fRescan) {
            pwalletMain->ScanForWalletTransactions(pindexGenesisBlock, true);
            pwalletMain->ReacceptWalletTransactions();
        }
    }

    return Value::null;
}

Value dumpprivkey(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "dumpprivkey <fuguecoinaddress>\n"
            "Reveals the private key corresponding to <fuguecoinaddress>.");

    string strAddress = params[0].get_str();
    CBitcoinAddress address;
    if (!address.SetString(strAddress))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Fuguecoin address");
    CKeyID keyID;
    if (!address.GetKeyID(keyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to a key");
    CSecret vchSecret;
    bool fCompressed;
    if (!pwalletMain->GetSecret(keyID, vchSecret, fCompressed))
        throw JSONRPCError(RPC_WALLET_ERROR, " PRIvate key for address " + strAddress + " is not known");
    return CBitcoinSecret(vchSecret, fCompressed).ToString();
}

Value dumpallprivkeys(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "dumpallprivkeys\n"
            "Reveals all private keys.");

    if (pwalletMain->IsLocked())
        throw JSONRPCError(-13, "Error: Please enter the wallet passphrase with walletpassphrase first.");
    Array ret;
    BOOST_FOREACH(const PAIRTYPE(CBitcoinAddress, string)& item, pwalletMain->mapAddressBook)
    {
        const CBitcoinAddress& address = item.first;
        const string& strName = item.second;

        CKeyID keyID;
        if (!address.GetKeyID(keyID))
           continue;
        CKey vchSecret;
        if (!pwalletMain->GetKey(keyID, vchSecret))
           continue;
        bool fCompressed = vchSecret.IsCompressed();
        CSecret vch_secret = vchSecret.GetSecret(fCompressed);
        ret.push_back(strprintf(
                          "address: %s (%s) key: %s",
                          address.ToString().c_str(),
                          strName.c_str(),
                          CBitcoinSecret(vch_secret, fCompressed).ToString().c_str()));
    }

    return ret;

}

/*
Value dumpbootstrap(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 3)
        throw runtime_error(
            "dumpbootstrap <destination> <endblock> [startblock=0]\n"
            "Creates a bootstrap format block dump of the blockchain in destination, which can be a directory or a path with filename, up to the given endblock number.\n"
            "Optional <startblock> is the first block number to dump.");

    string strDest = params[0].get_str();
    int nEndBlock = params[1].get_int();
    if (nEndBlock < 0 || nEndBlock > nBestHeight)
        throw runtime_error("End block number out of range.");

    int nStartBlock = 0;
    if (params.size() > 2)
        nStartBlock = params[2].get_int();
    if (nStartBlock < 0 || nStartBlock > nEndBlock)
        throw runtime_error("Start block number out of range.");

    boost::filesystem::path pathDest(strDest);
    if (boost::filesystem::is_directory(pathDest))
        pathDest /= "bootstrap.dat";

    try {
        FILE* file = fopen(pathDest.string().c_str(), "wb");
        if (!file)
            throw JSONRPCError(-1, "Error: Could not open bootstrap file for writing.");

        CAutoFile fileout = CAutoFile(file, SER_DISK, CLIENT_VERSION);
        if (!fileout)
            throw JSONRPCError(-1, "Error: Could not open bootstrap file for writing.");

        unsigned char pchMessageStart[4];
        GetMessageStart(pchMessageStart, true);

        for (int nHeight = nStartBlock; nHeight <= nEndBlock; nHeight++)
        {
            CBlock block;
            CBlockIndex* pblockindex = FindBlockByHeight(nHeight);
            block.ReadFromDisk(pblockindex, true);
            fileout << FLATDATA(pchMessageStart) << fileout.GetSerializeSize(block) << block;
        }

    } catch(const boost::filesystem::filesystem_error &e) {
        throw JSONRPCError(-1, "Error: Bootstrap dump failed!");
    }

    return "bootstrap file created";
}
*/

Value dumpbootstrap(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 3)
        throw runtime_error(
            "dumpbootstrap <destination> <endblock> [startblock=0]\n"
            "Creates a bootstrap format block dump of the blockchain in destination, which can be a directory or a path with filename, up to the given endblock number.\n"
            "Optional <startblock> is the first block number to dump.");

    string strDest = params[0].get_str();
    // int nEndBlock = params[1].get_int();
    // if (nEndBlock < 0 || nEndBlock > nBestHeight)
    //     throw runtime_error("End block number out of range.");
    int nEndBlock = 2725960; // 3rd Feb 2019
    if (params.size() > 1)
        nEndBlock = params[1].get_int();
    if (nEndBlock < 0 || nEndBlock > nBestHeight)
        throw runtime_error("End block number out of range.");
    int nStartBlock = 0;
    if (params.size() > 2)
        nStartBlock = params[2].get_int();
    if (nStartBlock < 0 || nStartBlock > nEndBlock)
        throw runtime_error("Start block number out of range.");

    unsigned char pchMessageStart[4] = { 0xf9, 0xbe, 0xb4, 0xd4 };

    boost::filesystem::path pathDest(strDest);
    if (boost::filesystem::is_directory(pathDest))
        pathDest /= "bootstrap.dat";

    try {
        FILE* file = fopen(pathDest.string().c_str(), "wb");
        if (!file)
            throw JSONRPCError(RPC_MISC_ERROR, "Error: Could not open bootstrap file for writing.");

        CAutoFile fileout = CAutoFile(file, SER_DISK, CLIENT_VERSION);
        if (!fileout)
            throw JSONRPCError(RPC_MISC_ERROR, "Error: Could not open bootstrap file for writing.");

        for (int nHeight = nStartBlock; nHeight <= nEndBlock; nHeight++)
        {
            CBlock block;
            CBlockIndex* pblockindex = FindBlockByHeight(nHeight);
            block.ReadFromDisk(pblockindex);
            fileout << FLATDATA(pchMessageStart) << fileout.GetSerializeSize(block) << block;
        }
    } catch(const boost::filesystem::filesystem_error &e) {
        throw JSONRPCError(RPC_MISC_ERROR, "Error: Bootstrap dump failed!");
    }

    // return NullUniValue;
    return "bootstrap file created";
}

Value linearizehashes(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 3)
        throw runtime_error(
            "linearizehashes <destination> <endblock>  [startblock=0]\n"
            "Creates a dump of linearized block hashes in destination, which can be a directory or a path with filename, up to the given endblock number.\n"
            "Optional <startblock> is the first block number to dump.");

    string strDest = params[0].get_str();

    int nEndBlock = 1646900; // 3rd Feb 2019
    if (params.size() > 1)
        nEndBlock = params[1].get_int();
    if (nEndBlock < 0 || nEndBlock > nBestHeight)
        throw runtime_error("End block number out of range.");

    int nStartBlock = 0;
    if (params.size() > 2)
        nStartBlock = params[2].get_int();
    if (nStartBlock < 0 || nStartBlock > nEndBlock)
        throw runtime_error("Start block number out of range.");

    boost::filesystem::path pathDest(strDest);
    if (boost::filesystem::is_directory(pathDest))
        pathDest /= "hashlist.txt";

    try {
        FILE* file = fopen(pathDest.string().c_str(), "w");
        if (!file)
            throw JSONRPCError(-1, "Error: Could not open output file for writing.");

        CAutoFile fileout = CAutoFile(file, SER_DISK, CLIENT_VERSION);
        if (!fileout)
            throw JSONRPCError(-1, "Error: Could not open output file for writing.");

        for (int nHeight = nStartBlock; nHeight <= nEndBlock; nHeight++)
        {
            CBlock block;
            CBlockIndex* pblockindex = FindBlockByHeight(nHeight);
            block.ReadFromDisk(pblockindex);
            std::string blockhash = block.GetHash().ToString().c_str();
            fileout << blockhash.append("\n");
        }
    } catch(const boost::filesystem::filesystem_error &e) {
        throw JSONRPCError(-1, "Error: Linearized hash dump failed!");
    }

    return "file of linearized hashes created";
}

