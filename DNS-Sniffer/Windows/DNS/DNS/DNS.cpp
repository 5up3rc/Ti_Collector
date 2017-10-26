// DNS.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
//#include <regex>
#include <pcap.h>
#include <string>
#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable:4996)

using namespace std;

typedef struct ip_address{
	u_char byte1;
	u_char byte2;
	u_char byte3;
	u_char byte4;
}ip_address;

/* IPv4 �ײ� */
typedef struct ip_header{
	u_char  ver_ihl;        // �汾 (4 bits) + �ײ����� (4 bits)
	u_char  tos;            // ��������(Type of service) 
	u_short tlen;           // �ܳ�(Total length) 
	u_short identification; // ��ʶ(Identification)
	u_short flags_fo;       // ��־λ(Flags) (3 bits) + ��ƫ����(Fragment offset) (13 bits)
	u_char  ttl;            // ���ʱ��(Time to live)
	u_char  proto;          // Э��(Protocol)
	u_short crc;            // �ײ�У���(Header checksum)
	ip_address  saddr;      // Դ��ַ(Source address)
	ip_address  daddr;      // Ŀ�ĵ�ַ(Destination address)
	u_int   op_pad;         // ѡ�������(Option + Padding)
}ip_header;

typedef struct dns_header{
	u_short id;
	u_short flags;
	u_short num_q;
	u_short num_answ_rr;
	u_short num_auth_rr;
	u_short num_addi_rr;
	u_char dnsname;
}dns_header;

/* UDP �ײ�*/
typedef struct udp_header{
	u_short sport;          // Դ�˿�(Source port)
	u_short dport;          // Ŀ�Ķ˿�(Destination port)
	u_short len;            // UDP���ݰ�����(Datagram length)
	u_short crc;            // У���(Checksum)
}udp_header;

int k = 0;
/* �ص�����ԭ�� */
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data);



int _tmain(int argc, _TCHAR* argv[])
{

	u_int netmask;
	pcap_if_t * allAdapters;//�������б�
	pcap_if_t * adapter;
	pcap_t           * adapterHandle;//���������
	//struct pcap_pkthdr * packetHeader;
	//const u_char       * packetData;
	char errorBuffer[PCAP_ERRBUF_SIZE];//������Ϣ������
	char packet_filter[] = "udp port 53";
	struct bpf_program fcode;
	if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL,
		&allAdapters, errorBuffer) == -1)
	{//�����������ӵ���������������
		fprintf(stderr, "Error in pcap_findalldevs_ex function: %s\n", errorBuffer);
		system("pause");
		return -1;
	}
	if (allAdapters == NULL)
	{//�������κ�������
		printf("\nNo adapters found! Make sure WinPcap is installed.\n");
		system("pause");
		return 0;
	}
	int crtAdapter = 0;
	for (adapter = allAdapters; adapter != NULL; adapter = adapter->next)
	{//����������������Ϣ(���ƺ�������Ϣ)
		printf("\n%d.%s ", ++crtAdapter, adapter->name);
		printf("-- %s\n", adapter->description);
	}
	printf("\n");
	//ѡ��Ҫ�������ݰ���������
	int adapterNumber;
	printf("Enter the adapter number between 1 and %d:", crtAdapter);
	scanf_s("%d", &adapterNumber);
	if (adapterNumber < 1 || adapterNumber > crtAdapter)
	{
		printf("\nAdapter number out of range.\n");
		// �ͷ��������б�
		pcap_freealldevs(allAdapters);
		system("pause");
		return -1;
	}
	adapter = allAdapters;
	for (crtAdapter = 0; crtAdapter < adapterNumber - 1; crtAdapter++)
		adapter = adapter->next;
	// ��ָ��������
	adapterHandle = pcap_open(adapter->name, // name of the adapter
		65536,         // portion of the packet to capture
		// 65536 guarantees that the whole 
		// packet will be captured
		PCAP_OPENFLAG_PROMISCUOUS, // promiscuous mode
		1000,             // read timeout - 1 millisecond
		NULL,          // authentication on the remote machine
		errorBuffer    // error buffer
		);
	if (adapterHandle == NULL)
	{//ָ����������ʧ��
		fprintf(stderr, "\nUnable to open the adapter\n", adapter->name);
		// �ͷ��������б�
		pcap_freealldevs(allAdapters);
		system("pause");
		return -1;
	}

	/* ���������·�㣬Ϊ�˼򵥣�����ֻ������̫�� */
	if (pcap_datalink(adapterHandle) != DLT_EN10MB)
	{
		fprintf(stderr, "\nThis program works only on Ethernet networks.\n");
		/* �ͷ��豸�б� */
		pcap_freealldevs(allAdapters);
		system("pause");
		return -1;
	}

	if (adapter->addresses != NULL)
		/* ��ýӿڵ�һ����ַ������ */
		netmask = ((struct sockaddr_in *)(adapter->addresses->netmask))->sin_addr.S_un.S_addr;
	else
		/* ����ӿ�û�е�ַ����ô���Ǽ���һ��C������� */
		netmask = 0xffffff;


	//���������
	if (pcap_compile(adapterHandle, &fcode, packet_filter, 1, netmask) <0)
	{
		fprintf(stderr, "\nUnable to compile the packet filter. Check the syntax.\n");
		/* �ͷ��豸�б� */
		pcap_freealldevs(allAdapters);
		system("pause");
		return -1;
	}

	//���ù�����
	if (pcap_setfilter(adapterHandle, &fcode)<0)
	{
		fprintf(stderr, "\nError setting the filter.\n");
		/* �ͷ��豸�б� */
		pcap_freealldevs(allAdapters);
		system("pause");
		return -1;
	}

	printf("\nlistening on %s...\n", adapter->description);

	/* �ͷ��豸�б� */
	pcap_freealldevs(allAdapters);

	/* ��ʼ��׽ */
	pcap_loop(adapterHandle, 0, packet_handler, NULL);

	return 0;
}
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
	//regex pattern("[a-zA-Z0-9][-a-zA-Z0-9]{0,62}(\.[a-zA-Z0-9][-a-zA-Z0-9]{0,62})+\.?", regex_constants::extended);
	//match_results<string::const_iterator> result;


	struct tm *ltime;
	struct dns_header *dns_protocol;
	char timestr[16];
	ip_header *ih;
	udp_header *uh;
	u_int ip_len;
	u_short sport, dport;
	time_t local_tv_sec;
	int u, v, w, y;

	/* ��ʱ���ת���ɿ�ʶ��ĸ�ʽ */
	local_tv_sec = header->ts.tv_sec;
	ltime = localtime(&local_tv_sec);
	strftime(timestr, sizeof timestr, "%H:%M:%S", ltime);
	//printf("%s.%.6d len:%d ", timestr, header->ts.tv_usec, header->len);

	/* ���IP���ݰ�ͷ����λ�� */
	ih = (ip_header *)(pkt_data +
		14); //��̫��ͷ������

	/* ���UDP�ײ���λ�� */
	ip_len = (ih->ver_ihl & 0xf) * 4;
	uh = (udp_header *)((u_char*)ih + ip_len);

	/* ָ��dns�����ֶ� */
	dns_protocol = (struct dns_header*)(pkt_data + 34 + 8);

	/* �������ֽ�����ת���������ֽ����� */
	sport = ntohs(uh->sport);
	dport = ntohs(uh->dport);

	//FILE *fp;
	//fp = fopen(".\\Dns.txt", "a");
	freopen(".\\Dns.txt", "a+", stdout);

	if (k == 0)
	{
		printf(" \n  IP Source          Dns Server             Time            DNS  \n  ");
	}
	k = 1;

	u = ntohs(dns_protocol->num_addi_rr);
	v = ntohs(dns_protocol->num_answ_rr);
	w = ntohs(dns_protocol->num_auth_rr);

	y = (!u) && (!v) && (!w);
	if ((!(ntohs(dns_protocol->flags) >> 15)) && y)//����DNS�����
		/* ��ӡIP��ַ��UDP�˿� */
	{

		printf("\n%d.%d.%d.%d.%d \t %d.%d.%d.%d.%d",

			ih->saddr.byte1,
			ih->saddr.byte2,
			ih->saddr.byte3,
			ih->saddr.byte4,
			sport,
			ih->daddr.byte1,
			ih->daddr.byte2,
			ih->daddr.byte3,
			ih->daddr.byte4,
			dport);

		u_char *query = &(dns_protocol->dnsname);
		u_char  domainname[5120] = { 0 };

		u_int k = 0;
		query++;

		while (*query)
		{
			if (*query < 0x10)//48�Ժ�������ֺ�Ӣ����ĸ  
			{

				domainname[k] = '.';
			}
			else
			{
				domainname[k] = *query;
			}

			query++;
			k++;

		}
		/* ��ӡ���ݰ���ʱ����ͳ��� */

		printf("\t\t%s.%.6d  ", timestr, header->ts.tv_usec);
		//string buf;
		//buf = (char*)domainname;
		//bool valid = regex_match(buf, result, pattern);
		//if (!valid)
		//	fprintf(fp, "\tUknown");
		//else
		printf("\t%s", domainname);

		fflush(stdout);


		//fclose(fp);
	}
}

