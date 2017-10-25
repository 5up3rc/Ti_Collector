#include <stdio.h>
#include <unistd.h>
#include <pcap/pcap.h>
#include <arpa/inet.h>
#include <time.h>

typedef struct iphdr_t {
	u_int8_t ver_ihl;
	u_int8_t tos;
	u_int16_t tot_len;
	u_int16_t id;
	u_int16_t frag_off;
   	u_int8_t ttl;
   	u_int8_t protocol;
    u_int16_t check;
    u_int32_t saddr;
    u_int32_t daddr;
    u_int32_t op_pad;
   
}ip_header;

typedef struct udp_header{
    u_int16_t sport;          // Դ�˿�(Source port)
    u_int16_t dport;          // Ŀ�Ķ˿�(Destination port)
    u_int16_t len;            // UDP���ݰ�����(Datagram length)
    u_int16_t crc;            // У���(Checksum)
}udp_header;

typedef struct dns_packet //����head+data  
{  
      
    u_int16_t id;//ÿһ��ռ2���ֽڣ���12���ֽ�  
    u_int16_t flags;//��־��һ��Ϊ0�����ѯ����  
    u_int16_t ques;  
    u_int16_t answer;  
    u_int16_t author;  
    u_int16_t addition;  
    u_int8_t dns_data;//��ѯ���ⲿ��  
  
}dns_packet; 

int g=0;
void packet_handler(u_char *arg, const struct pcap_pkthdr *pkthdr, const u_char *raw_data);

int main(void)
{
    pcap_t *pt;
    char *dev;
    char errbuf[128];
    struct bpf_program fp;
    bpf_u_int32 maskp, netp;
    int ret,i=0;
	int inum;
    char filter[128] = "udp port 53";
    pcap_if_t *alldevs,*d;

    if(pcap_findalldevs(&alldevs, errbuf) == -1) {
        fprintf(stderr,"find interface failed!\n");
        return -1;
    }

    for(d = alldevs; d; d = d->next) {
        printf("%02d. %s", ++i, d->name);
        if(d->description)
            printf("(%s)\n", d->description);
        else
            printf("(no description available)\n");
    }

	if(i == 1) {
		dev = alldevs->name;
	} else {
		printf("input a interface(1-%d):", i);
		scanf("%d", &inum);
		if(inum < 1 || inum > i) {
			printf("interface number out of range\n");
			return -1;
		}

		for(d = alldevs, i = 1; i<inum; d = d->next, i++);

		dev = d->name;
	}

	
	dev=pcap_lookupdev(errbuf);
	if(dev==NULL){
		 printf("%s/n",errbuf);
		 return -1;
	}
	
	/*dev = "any";*/

    printf("dev: %s\n", dev);
    ret = pcap_lookupnet(dev, &netp, &maskp, errbuf);
    if(ret == -1) {
		
        printf("%s\n", errbuf);
   	    return -1;
    }

    pt = pcap_open_live(dev, 65535, 1, 0, errbuf);
    if(pt == NULL)
	{	
        printf("open error :%s\n", errbuf);
        return -1;
    }
    
    if(pcap_compile(pt, &fp, filter, 0, netp) == -1) {
        printf( "compile error\n");
        return -1;
    }

    if(pcap_setfilter(pt, &fp) == -1) {
        printf("setfilter error\n");
        return -1;
    }
   
	pcap_loop(pt, -1, packet_handler, NULL);

pcap_close(pt);
   
	return 0;
}

void packet_handler(u_char *arg, const struct pcap_pkthdr *pkthdr, const u_char *pkt_data)
{
    struct tm *ltime; //����ʱ��  
  
    char timestr[16];   
  
    ip_header *ih;   
  
    udp_header *uh;   
  
    u_int16_t sport,dport;   
  
    time_t local_tv_sec;   
    
    struct in_addr s,d;	
 
    int u,v,w,y;	
   // FILE* f;


   // f=fopen("./dns.txt","a+");
    freopen("dns.txt","a+",stdout);
//    if(f==NULL)
//	{
//		printf("error");
//		exit(1);
//	}
    /* ��ʱ���ת���ɿ�ʶ��ĸ�ʽ */ //....������ת��....  
    local_tv_sec = pkthdr->ts.tv_sec;   
    ltime=localtime(&local_tv_sec);   
    strftime( timestr, sizeof timestr, "%H:%M:%S", ltime);   
  
  
  
    /* ���IP���ݰ�ͷ����λ�� */   
    ih = (ip_header *) (pkt_data + 14); //��̫��ͷ������  
  
    /* ���UDP�ײ���λ�� */   
  
    uh = (udp_header *) ( pkt_data + 34);  
  
  
    /* �������ֽ�����ת���������ֽ����� */  
    sport = ntohs( uh->sport );   
    dport = ntohs( uh->dport );
    
	s.s_addr=ih->saddr;
	d.s_addr=ih->daddr;
//	fprintf(f,"\nIP Source: %s ->",inet_ntoa(s));
//	printf("\nIP Source: %s ->",inet_ntoa(s));
//	printf("Dns Server: %s",inet_ntoa(d));
//	printf("Dns Server: %s\t",inet_ntoa(d));

    struct dns_packet *pdns;  
    pdns = (struct dns_packet *)(pkt_data+34+8); // sport+dport+length+checksum,DNSͷָ��  
  
    u_char *query=&(pdns->dns_data);//��λ����ѯ����ͷ��  
  
	u_char domainname[5120]={0};  
  
    u_int32_t i=0;  
    query++;//�ѵ�ȥ��  
  
    while(*query)  
    {  
      
        if(*query < 0x10)//48�Ժ�������ֺ�Ӣ����ĸ  
        {  
  
            domainname[i]='.';  
        }  
        else  
        {  
            domainname[i]=*query;  
        }  
  
        query++;  
        i++;  
    }
    if(g==0)
	{
		printf("IP Source    DNS Server   Time  Dns");
	}
    g=1;
    
	u=ntohs(pdns->answer);
	v=ntohs(pdns->author);
	w=ntohs(pdns->addition);

    y=(!u)&&(!v)&&(!w);



    if ((!(ntohs(pdns->flags)>>15))&&y)
	{
	  
		printf("\n %s\t\t",inet_ntoa(s));
		printf(" %s\t",inet_ntoa(d));
		printf("  %s.%.2d ", timestr, pkthdr->ts.tv_usec); //..........��ʾʱ������ݰ�����..........��ʾ2  
	  	printf("\t %s",domainname);
	   	fflush(stdout);
	}
  
}
