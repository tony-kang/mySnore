#include <QtCore/QCoreApplication>
#include <qfile.h>
#include <qtextstream.h>
#include <QDateTime>
#include <iostream>
#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
using namespace std;
unsigned char temp[1024];

typedef struct {
	unsigned int sign;    //0x55aa55aa
	unsigned int imgAver;
	unsigned int imgBver;
	unsigned int upgradeType;  //0x0a for image A ; 0x0b for image B; 0x00 no upgrade
}system_parameter_bak_def;

system_parameter_bak_def  SystemPara;

void InitSystemPara(int ver)
{
	SystemPara.sign = 0x55aa55aa;
	SystemPara.imgAver = ver;
	SystemPara.imgBver = 0;
	SystemPara.upgradeType = ver;

}


int addFile(QFile *file1, QFile *file2)
{
	file1->open(QIODevice::WriteOnly);
	file2->open(QIODevice::ReadOnly);
	QByteArray bytes = file2->readAll();
	file1->write((char*)bytes.data(), bytes.size());
	file1->close();
	file2->close();

	return 1;
}

void SecurityImage(QString *strfile1, QString *strfile2, unsigned char ver1, unsigned char ver2, unsigned char ver3, unsigned char type)
{
	unsigned char temp[32], *pt;
	unsigned int i, cnt;
	unsigned short checksum = 0;
	QFile file1(*strfile1);
	QFile file2(*strfile2);
	file1.open(QIODevice::WriteOnly);
	file2.open(QIODevice::ReadOnly);
	QByteArray bytes = file2.readAll();
	pt = (unsigned char*)bytes.data();
	cnt = bytes.size();
	for (i = 0; i<cnt; i++)
		checksum += *pt++;
	memset(temp, 0, 32);
	temp[0] = 0xaa;
	temp[1] = 0x55;
	temp[2] = 0xaa;
	temp[3] = 0x55;

	temp[4] = ver3;
	temp[5] = ver2;
	temp[6] = ver1;

	temp[7] = type;

	temp[8] = cnt & 0xff;
	temp[9] = (cnt & 0xff00) >> 8;
	temp[10] = (cnt & 0xff0000) >> 16;
	temp[11] = (cnt & 0xff000000) >> 24;

	temp[12] = checksum & 0xff;
	temp[13] = (checksum & 0xff00) >> 8;

	checksum = 0;

	for (i = 0; i<32; i++)
		checksum += temp[i];

	temp[31] = checksum & 0xff;

	for (i = 0; i<32; i++)
		temp[i] = (~temp[i]) ^ 0x55;


	file1.write((char*)temp, 32);

	file1.write((char*)bytes.data(), bytes.size());
	file1.close();
	file2.close();

}



int main(int argc, char *argv[])
{
	int i, size;
	bool ok, pillow3 = false, mat = false, blanket = false;
	QString bootloader, imagea, imageb, ver1, ver2, ver3;
	QCoreApplication a(argc, argv);
	char * buffer = new char[2];
	bootloader = argv[1];
	imagea = argv[2];
	imageb = argv[3];
	ver1 = argv[4];
	ver2 = argv[5];
	ver3 = argv[6];
	cout << argv[1] << "  " << argv[2] << "  " << argv[3] << "  " << argv[4] << "  " << argv[5] << "  " << argv[6] << "\n";
	QFile headFile("..\\MP2_useLIB\\Inc\\tuwan.h");
	if (headFile.open(QIODevice::ReadOnly)) {
		headFile.read(buffer, 3);

		if ((buffer[0] == '/') && (buffer[1] == '/') && (buffer[2] == '/'))
			mat = true;
		else if ((buffer[0] == '/') && (buffer[1] == '/'))
			blanket = true;
		else    pillow3 = true;
		headFile.close();
	}
	else cout << "open file failure\n";
	QFile BootloaderFile(bootloader);
	QFile ImageAfile(imagea);

	QString fileName;
	QString NewImageA;
	QString NewImageB;

	if (pillow3)
	{
		fileName = "MP1_PLUS_V" + ver1 + "." + ver2 + "." + ver3 + ".bin";
		NewImageA = "MP1_PLUS_A_" + ver1 + "." + ver2 + "." + ver3 + ".rom";
		NewImageB = "MP1_PLUS_B_" + ver1 + "." + ver2 + "." + ver3 + ".rom";


	}
	else if (blanket)
	{
		fileName = "tuwan3gV" + ver1 + "." + ver2 + "." + ver3 + ".bin";
		NewImageA = "TUWAN3G_A_" + ver1 + "." + ver2 + "." + ver3 + ".rom";
		NewImageB = "TUWAN3G_B_" + ver1 + "." + ver2 + "." + ver3 + ".rom";
	}
	else
	{
		fileName = "tuwan2gV" + ver1 + "." + ver2 + "." + ver3 + ".bin";
		NewImageA = "TUWAN2G_A_" + ver1 + "." + ver2 + "." + ver3 + ".rom";
		NewImageB = "TUWAN2G_B_" + ver1 + "." + ver2 + "." + ver3 + ".rom";


	}
	QFile Newfile(fileName);
	addFile(&Newfile, &BootloaderFile);
	Newfile.open(QIODevice::Append);
	memset(temp, 0, 1024);
	size = Newfile.size();
	for (i = 0; i<(64 * 1024 - size); i++)
		Newfile.write((char*)temp, 1);


	InitSystemPara(ver3.toUInt(0, 10));
	memset(temp, 0, 1024);
	memcpy(temp, &SystemPara, sizeof(system_parameter_bak_def));

	for (i = 0; i<64; i++)
	{
		Newfile.write((char*)temp, 1024);
		memset(temp, 0, 1024);
	}

	ImageAfile.open(QIODevice::ReadOnly);
	QByteArray bytes = ImageAfile.readAll();
	Newfile.write((char*)bytes.data(), bytes.size());

	memset(temp, 0, 1024);
	size = ImageAfile.size();
	for (i = 0; i<(384 * 1024 - size); i++)
		Newfile.write((char*)temp, 1);

	for (i = 0; i<(384 * 1024); i++)
		Newfile.write((char*)temp, 1);
	for (i = 0; i<(128 * 1024); i++)
		Newfile.write((char*)temp, 1);
	Newfile.close();
	ImageAfile.close();


	SecurityImage(&NewImageA, &imagea, ver1.toUInt(&ok, 10), ver2.toUInt(&ok, 10), ver3.toUInt(&ok, 10), 1);
	SecurityImage(&NewImageB, &imageb, ver1.toUInt(&ok, 10), ver2.toUInt(&ok, 10), ver3.toUInt(&ok, 10), 2);


//	return a.exec();
	return 1;

}
