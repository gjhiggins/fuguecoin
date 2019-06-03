#!/usr/bin/env python3
#
# linearize-hashes.py:  List blocks in a linear, no-fork version of the chain.
#
# Copyright (c) 2013-2016 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
#
import unittest
import os
from datetime import datetime, timedelta, tzinfo
import binascii
import json
import subprocess
import re
import requests
import time
from rdflib import (
    Namespace,
    URIRef,
    Graph,
    Literal,
)
from rdflib.namespace import RDF, XSD, SKOS  # , RDFS
import base64
import sys

settings = dict(
    rpcuser="fuguecoinrpcuser",
    rpcpassword="fuguecoinrpcuserpassword",
    host="127.0.0.1",
    port=9088,
    rpcport=9089,
    min_height=0,
    max_height=2374500,
    netmagic=0xf9beb4d4,
    genesis=0x0000008625534e38467b29078ed2868c2ec43eb024c4a2e7ff2f6480f283656b,
    input="/home/gjh/minkiz/fabshop/FugueCoinWork/fuguecoin/datadir/blocks",
    output="/home/gjh/minkiz/fabshop/FugueCoinWork/fuguecoin-0.8.6/datadir/bootstrap.dat",
    hashlist="hashlist.txt",
    split_year=5,
    out_of_order_cache_sz = 100000000
)


# Switch endian-ness
def hex_switchEndian(s):
    """ Switches the endianness of a hex string (in pairs of hex chars) """
    pairList = [s[i:i + 2].encode() for i in range(0, len(s), 2)]
    return b''.join(pairList[::-1]).decode()


class RPCHost(object):
    def __init__(self, url):
        self._session = requests.Session()
        self._url = url
        self._headers = {'content-type': 'application/json'}

    def call(self, rpcMethod, *params):
        payload = json.dumps({"method": rpcMethod, "params": list(params), "jsonrpc": "2.0"})
        tries = 10
        hadConnectionFailures = False
        while True:
            # print("{url} {headers} {data}".format(url=self._url, headers=self._headers, data=payload))
            try:
                response = self._session.get(self._url, headers=self._headers, data=payload)
            except requests.exceptions.ConnectionError:
                tries -= 1
                if tries == 0:
                    raise Exception('Failed to connect for remote procedure call.')
                hadConnectionFailures = True
                print("Couldn't connect for remote procedure call, will sleep for ten seconds and then try again ({} more tries)".format(tries))
                time.sleep(10)
            else:
                if hadConnectionFailures:
                    print('Connected for remote procedure call after retry.')
                break
        if response.status_code not in (200, 500):
            raise Exception('RPC connection failure: ' + str(response.status_code) + ' ' + response.reason)
        responseJSON = response.json()
        if 'error' in responseJSON and responseJSON['error'] is not None:
            raise Exception('Error in RPC call: ' + str(responseJSON['error']))
        return responseJSON['result']


class TestMyView(unittest.TestCase):
    def setUp(self):
        self.serverurl = 'http://{}:{}@localhost:{}/'.format(
            settings.get('rpcuser'), settings.get('rpcpassword'), settings.get('rpcport'))
        self.amerpc = RPCHost(self.serverurl)

    @unittest.skip("Passed, skipping")
    def test_get_block_hashes(self):
        with open('/home/gjh/minkiz/fabshop/FugueCoinWork/fuguecoin-0.8.6/contrib/linearize/fuguehashes.txt', 'w') as fp:
            for height in range(0, 2374500):
                res = hex_switchEndian(self.amerpc.call('getblockhash', height))
                fp.write("{}\n".format(res))
            fp.close()

    def test_get_unswitch(self):
        with open(
            '/home/gjh/minkiz/fabshop/FugueCoinWork/fuguecoin-0.8.6/contrib/linearize/fuguehashes.txt', 'r') as fin, open(
            '/home/gjh/minkiz/fabshop/FugueCoinWork/fuguecoin-0.8.6/contrib/linearize/hashlist.txt', 'w') as fout:
            for l in fin.readlines():
                res = hex_switchEndian(l[:-1])
                fout.write("{}\n".format(res))
            fout.close()

if __name__ == "__main__":
    unittest.main()
