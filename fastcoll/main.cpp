/*

MD5 collision generator
=======================
Source code files:
  block0.cpp
  block1.cpp
  main.cpp
  main.hpp
  md5.cpp
  block1wang.cpp
  block1stevens00.cpp
  block1stevens01.cpp
  block1stevens10.cpp
  block1stevens11.cpp
Win32 executable:
  fastcoll_v1.0.0.5.exe

Version
=======
version 1.0.0.5, April 2006.

Copyright
=========
� M. Stevens, 2006. All rights reserved.

Disclaimer
==========
This software is provided as is. Use is at the user's risk.
No guarantee whatsoever is given on how it may function or malfunction.
Support cannot be expected.
This software is meant for scientific and educational purposes only.
It is forbidden to use it for other than scientific or educational purposes.
In particular, commercial and malicious use is not allowed.
Further distribution of this software, by whatever means, is not allowed
without our consent.
This includes publication of source code or executables in printed form,
on websites, newsgroups, CD-ROM's, etc.
Changing the (source) code without our consent is not allowed.
In all versions of the source code this disclaimer, the copyright
notice and the version number should be present.

*/

#include <iostream>
#include <fstream>
#include <time.h>

#include "main.hpp"

using namespace std;

const uint32 MD5IV[] = { 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476 };

unsigned load_block(istream& i, uint32 block[]);
void save_block(ostream& o, const uint32 block[]);
void find_collision(const uint32 IV[], uint32 msg1block0[], uint32 msg1block1[], uint32 msg2block0[], uint32 msg2block1[], bool verbose = false);


#include <sstream>
#include <string>
#include <utility>
//#include <boost/filesystem/operations.hpp>
//#include <boost/program_options.hpp>
//#include <boost/lexical_cast.hpp>
//#include <boost/timer.hpp>
//#include <boost/cstdint.hpp>

//typedef boost::uint64_t uint64;

//namespace fs = boost::filesystem;
//namespace po = boost::program_options;
//using boost::lexical_cast;

//void test_md5iv(bool single = false);
//void test_rndiv(bool single = false);
//void test_reciv(bool single = false);
//void test_all();

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("Usage: %s <file_name>\n", argv[0]);
        return 0;
    }

	seed32_1 = uint32(time(NULL));
	seed32_2 = 0x12345678;

	uint32 IV[4] = { MD5IV[0], MD5IV[1], MD5IV[2], MD5IV[3] };

	string outfn1 = "msg1.jpg";
	string outfn2 = "msg2.jpg";
	string ihv;
	string prefixfn;

	cout <<
		"MD5 collision generator v1.5\n"
		"by Marc Stevens (http://www.win.tue.nl/hashclash/)\n"
		<< endl;

	try
	{

		ofstream ofs1(outfn1.c_str(), ios::binary);
		if (!ofs1)
		{
			cerr << "Error: cannot open outfile: '" << outfn1 << "'" << endl;
			return 1;
		}
		ofstream ofs2(outfn2.c_str(), ios::binary);
		if (!ofs2)
		{
			cerr << "Error: cannot open outfile: '" << outfn2 << "'" << endl;
			return 1;
		}

		{
            ifstream ifs(argv[1], ios::binary);
            if (!ifs)
            {
                cerr << "Error: cannot open inputfile: " << endl;
                return 1;
            }
            uint32 block[16];
            while (true)
            {
                unsigned len = load_block(ifs, block);
                if (len)
                {
                    save_block(ofs1, block);
                    save_block(ofs2, block);
                    md5_compress(IV, block);
                } else
                    break;
            }
		}


		uint32 msg1block0[16];
		uint32 msg1block1[16];
		uint32 msg2block0[16];
		uint32 msg2block1[16];
		find_collision(IV, msg1block0, msg1block1, msg2block0, msg2block1, true);

		save_block(ofs1, msg1block0);
		save_block(ofs1, msg1block1);
		save_block(ofs2, msg2block0);
		save_block(ofs2, msg2block1);
		return 0;
	} catch (exception& e)
	{
		cerr << "\nException caught:\n" << e.what() << endl;
		return 1;
	} catch (...)
	{
		cerr << "\nUnknown exception caught!" << endl;
		return 1;
	}
}


unsigned load_block(istream& i, uint32 block[])
{
	unsigned len = 0;
	char uc;
	for (unsigned k = 0; k < 16; ++k)
	{
		block[k] = 0;
		for (unsigned c = 0; c < 4; ++c)
		{
			i.get(uc);
			if (i) 
				++len;
			else
				uc = 0;
			block[k] += uint32((unsigned char)(uc))<<(c*8);
		}
	}
	return len;
}

void save_block(ostream& o, const uint32 block[])
{
	for (unsigned k = 0; k < 16; ++k)
		for (unsigned c = 0; c < 4; ++c)
			o << (unsigned char)((block[k] >> (c*8))&0xFF);
}

void find_collision(const uint32 IV[], uint32 msg1block0[], uint32 msg1block1[], uint32 msg2block0[], uint32 msg2block1[], bool verbose)
{
	if (verbose)
		cout << "Generating first block: " << flush;
	find_block0(msg1block0, IV);

	uint32 IHV[4] = { IV[0], IV[1], IV[2], IV[3] };
	md5_compress(IHV, msg1block0);

	if (verbose)
		cout << endl << "Generating second block: " << flush;
	find_block1(msg1block1, IHV);

	for (int t = 0; t < 16; ++t)
	{
		msg2block0[t] = msg1block0[t];
		msg2block1[t] = msg1block1[t];
	}
	msg2block0[4] += 1 << 31; msg2block0[11] += 1 << 15; msg2block0[14] += 1 << 31;
	msg2block1[4] += 1 << 31; msg2block1[11] -= 1 << 15; msg2block1[14] += 1 << 31;
	if (verbose)
		cout << endl;
}
