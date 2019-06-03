#define BOOST_TEST_MODULE Bitcoin Test Suite
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>

#include "db.h"
#include "txdb.h"
#include "main.h"
#include "wallet.h"
#include "util.h"

CWallet* pwalletMain;
CClientUIInterface uiInterface;

extern bool fPrintToConsole;
extern void noui_connect();

struct TestingSetup {
    CCoinsViewDB *pcoinsdbview;
    boost::filesystem::path pathTemp;
    boost::thread_group threadGroup;

    TestingSetup() {
        BOOST_TEST_MESSAGE("1");
        fPrintToDebugger = true; // don't want to write to debug.log file
        BOOST_TEST_MESSAGE("2");
        noui_connect();
        BOOST_TEST_MESSAGE("3");
        bitdb.MakeMock();
        BOOST_TEST_MESSAGE("4");
        pathTemp = GetTempPath() / strprintf("test_bitcoin_%lu_%i", (unsigned long)GetTime(), (int)(GetRand(100000)));
        boost::filesystem::create_directories(pathTemp);
        BOOST_TEST_MESSAGE("5");
        mapArgs["-datadir"] = pathTemp.string();
        pblocktree = new CBlockTreeDB(1 << 20, true);
        pcoinsdbview = new CCoinsViewDB(1 << 23, true);
        pcoinsTip = new CCoinsViewCache(*pcoinsdbview);
        // BOOST_TEST_MESSAGE("6");
        // InitBlockIndex();
        BOOST_TEST_MESSAGE("7");
        bool fFirstRun;
        pwalletMain = new CWallet("wallet.dat");
        BOOST_TEST_MESSAGE("8");
        pwalletMain->LoadWallet(fFirstRun);
        RegisterWallet(pwalletMain);
        BOOST_TEST_MESSAGE("19");
        nScriptCheckThreads = 3;
        for (int i=0; i < nScriptCheckThreads-1; i++)
            BOOST_TEST_MESSAGE("t1");
            threadGroup.create_thread(&ThreadScriptCheck);
    }
    ~TestingSetup()
    {
        threadGroup.interrupt_all();
        threadGroup.join_all();
        delete pwalletMain;
        pwalletMain = NULL;
        delete pcoinsTip;
        delete pcoinsdbview;
        delete pblocktree;
        bitdb.Flush(true);
        boost::filesystem::remove_all(pathTemp);
    }
};

BOOST_GLOBAL_FIXTURE(TestingSetup);

void Shutdown(void* parg)
{
  exit(0);
}

void StartShutdown()
{
  exit(0);
}

