// ConsoleApplication4.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

class InfoSet
{
public:
	struct infoStruct
	{
		int info[13];
		int len;
	};
	//InfoSet(int size, string content);
	void untruncate_data();
	void unscramble_data();
	string get_string();
public:
	infoStruct in;
};

//InfoSet::InfoSet(int size, string content)
//{
	//int i;

	//in.len = size;
	//for (i = 0;i < size;i++)
	//{
	//	in.info[i] = content.at(i);
	//}

	//while (i <= 12) in.info[i++] = rand() % 92 + 33;
//}
string InfoSet::get_string()
{
	string res;
	res.resize(in.len);
	for (int i = 0;i < in.len;i++)
	{
		res[i] = in.info[i];
	}
	return res;
}

void InfoSet::untruncate_data()
{
	for (int i = 0;i <= 12;i++)
	{
		in.info[i] -= rand() % 33333;
	}
}

void InfoSet::unscramble_data()
{
	int tmp;

	tmp = in.info[7];
	in.info[7] = in.info[0];
	in.info[0] = tmp;

	tmp = in.info[3];
	in.info[3] = in.info[2];
	in.info[2] = tmp;

	tmp = in.info[5];
	in.info[5] = in.info[9];
	in.info[9] = tmp;

	tmp = in.info[4];
	in.info[4] = in.info[8];
	in.info[8] = tmp;

	tmp = in.info[6];
	in.info[6] = in.info[1];
	in.info[1] = tmp;
}

string hdd_sn;
void init_hdd_sn(string crybuf)
{
	hdd_sn.resize(8);
	int len = crybuf.size();
	for (int i = 0;i <= 7;i++)
		hdd_sn[i] = crybuf[(i + 13) % len];
}

string baseboard_SN;
void init_baseboard_sn(string crybuf)
{
	baseboard_SN.resize(8);
	int len = crybuf.size();
	for (int i = 0;i <= 6;i++)
		baseboard_SN[i] = crybuf[(i + 35) % len];
}

void scramble_vects(vector<InfoSet>& pvec)
{
	int i;
	for (i = 0;i < pvec.size() / 12;i += 12)
	{
		InfoSet tmp = pvec[i + 7];
		pvec[i + 7] = pvec[i];
		pvec[i] = tmp;

		tmp = pvec[i + 6];
		pvec[i + 6] = pvec[i + 1];
		pvec[i + 1] = tmp;

		swap(pvec[i + 3], pvec[i + 2]);
		swap(pvec[i + 4], pvec[i + 8]);

		swap(pvec[i + 5], pvec[i + 9]);

		swap(pvec[i + 10], pvec[i + 11]);
	}

	if (pvec.size() % 12 > 3)
	{
		swap(pvec[i + 3], pvec[i + 2]);

		if (pvec.size() % 12 > 6)
		{
			swap(pvec[i + 6], pvec[i + 1]);

			if (pvec.size() % 12 > 7)
			{
				swap(pvec[i + 7], pvec[i + 0]);

				if (pvec.size() % 12 > 8)
				{
					swap(pvec[i + 4], pvec[i + 8]);

					if (pvec.size() % 12 > 9)
					{
						swap(pvec[i + 5], pvec[i + 9]);
					}
				}
			}
		}
	}
}

string getModifiedInfoString(InfoSet * infoset)
{
	string ret;

	int random_picked = rand();
	unsigned int deductedSize = infoset->in.len + 0xFFFFFFFE - random_picked % 11;
	infoset->unscramble_data();
	infoset->in.len = deductedSize;
	if (deductedSize > 13)
	{
		cout << "file invalid!!!!" << endl;
		return ret;
	}

	ret.resize(deductedSize);
	for (int i = 0;i < deductedSize;i++)
	{
		ret[i] = infoset->in.info[i];
	}
	return ret;
}

string unscramble_file(char* filename)
{
	vector<InfoSet> vec_info;
	InfoSet tmp;
	string ret;
	fstream input(filename, ios::in | ios::binary);
	if (!input.is_open())
	{
		cout << "open file failed!!!" << endl;
	}

	int magic;
	input.read((char*)&magic, 4);
	if (magic == 0x4B45BF3C)
	{
		int version;
		input.read((char*)&version, 4);
		if (version == 0x10200)
		{
			int visible_header_lenght;
			input.read((char*)&visible_header_lenght, 4);
			char visible_header[1600];
			input.read(visible_header, visible_header_lenght);

			while (!input.eof())
			{
				input.read((char*)&tmp, sizeof(tmp));
				vec_info.push_back(tmp);
			}

			input.close();
			scramble_vects(vec_info);
			int seed = 1618;
			for (int i = 0; i <= 7; ++i)
			{
				seed += hdd_sn[i];
			}
			srand(seed);

			for (int i = 0;i < vec_info.size();i++)
			{
				ret += getModifiedInfoString(&vec_info[i]);
			}

			int truncseed = 8161;
			for (int i = 0; i <= 6; ++i)
			{
				truncseed += baseboard_SN[i];
			}
			srand(truncseed);
			for (int i = 0;i < vec_info.size();i++)
			{
				vec_info[i].untruncate_data();
			}

			string ss;
			for (int i = 0;i < vec_info.size();i++)
			{
				ss += vec_info[i].get_string();
			}

			ret = ss;
		}
	}

	return ret;
}

void read_mrf(string crybuf)
{
	init_hdd_sn(crybuf);
	init_baseboard_sn(crybuf);
	string ret = unscramble_file("E:\\SN001638-20200304-100416.mlf");
}

int main()
{
	vector<InfoSet> vec_info;
	InfoSet tmp;
	string res;
	int len = 0;
	char crybuf[65];

	memset(crybuf, 0, 65);
	srand(0x9E4Bu);
	fstream input("E:\\SN001638-20200703-171037.mlr", ios::in | ios::binary);
	if (!input.is_open())
	{
		cout << "open file failed!!!" << endl;
	}

	while (!input.eof())
	{
		input.read((char*)&tmp, sizeof(tmp));
		vec_info.push_back(tmp);
	}

	input.close();

	for (int i = 0;i < vec_info.size();i++)
	{
		len += vec_info[i].in.len;
		vec_info[i].untruncate_data();
		vec_info[i].unscramble_data();
	}

	for (int i = 0;i < vec_info.size();i++)
	{
		res += vec_info[i].get_string();
	}

	memcpy(crybuf, res.c_str() + 109, 64);
	string src = crybuf;
	read_mrf(src);
	return 0;
}