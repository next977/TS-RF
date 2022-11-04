#include "usbotg.h"

typedef int (*_usbg_init)(const char*, usbg_state **);
typedef int (*_usbg_create_gadget)( usbg_state *, const char *, usbg_gadget_attrs* , usbg_gadget_strs* , usbg_gadget **);
typedef int (*_usbg_create_function)( usbg_gadget *, usbg_function_type , const char *, usbg_function_attrs *, usbg_function **);
typedef const char *(*(_usbg_error_name)(usbg_error));
typedef const char *(*(_usbg_strerror)(usbg_error));
typedef void (*_usbg_cleanup)(usbg_state*);
typedef void (*_usbg_free_gadget)(usbg_gadget*);
typedef int (*_usbg_create_config)(usbg_gadget *, int, const char *, usbg_config_attrs *, usbg_config_strs *, usbg_config **);
typedef int (*_usbg_add_config_function)(usbg_config *, const char *, usbg_function *);
typedef int (*_usbg_enable_gadget)(usbg_gadget *, const char *);
//typedef int (*_usbg_get_udcs)(struct dirent ***);
typedef struct usbg_udc *(*(_usbg_get_udc)(usbg_state *, const char *));
typedef int (*_usbg_write_string)(const char *, const char *, const char *, const char *);
typedef int (*_usbg_write_buf)(const char *, const char *, const char *, const char *);
typedef void (*_usbg_free_binding)(usbg_binding*);
typedef void (*_usbg_free_config)(usbg_config *);
typedef int (*_usbg_rm_binding)(usbg_binding*);
typedef int (*_usbg_set_net_dev_addr)(usbg_function* , struct ether_addr*);
typedef int (*_usbg_set_net_host_addr)(usbg_function *f, struct ether_addr *addr);

_usbg_init                  usbg_init_r = NULL;
_usbg_create_gadget         usbg_create_gadget_r = NULL;
_usbg_create_function       usbg_create_function_r = NULL;
_usbg_error_name*           usbg_error_name_r = NULL;
_usbg_strerror*             usbg_strerror_r = NULL;
_usbg_cleanup               usbg_cleanup_r = NULL;
_usbg_free_gadget           usbg_free_gadget_r = NULL;
_usbg_create_config         usbg_create_config_r = NULL;
_usbg_add_config_function   usbg_add_config_function_r = NULL;
_usbg_enable_gadget         usbg_enable_gadget_r = NULL;
//_usbg_get_udcs              usbg_get_udcs_r = NULL;
_usbg_get_udc*              usbg_get_udc_r = NULL;
_usbg_write_string          usbg_write_string_r = NULL;
_usbg_write_buf             usbg_write_buf_r = NULL;
_usbg_free_binding          usbg_free_binding_r = NULL;
_usbg_free_config           usbg_free_config_r = NULL;
_usbg_rm_binding            usbg_rm_binding_r = NULL;
_usbg_set_net_dev_addr      usbg_set_net_dev_addr_r = NULL;
_usbg_set_net_host_addr     usbg_set_net_host_addr_r = NULL;

QLibrary libusb;

QHostAddress* QHostAddr;
QList<QHostAddress> AddrList;
QHostInfo* info;
QNetworkAccessManager *networkManager;
QString DevAddr;
usbg_function * f_acm0;
usbg_function * f_eem0;
usbg_function * f_rndis0;
char *usb_uart = "/dev/ttyGS0";
int  uart_fd;

usbg_gadget_strs g_strs={
  "012345",
  "Starmed Inc",
  "Bar Gadget"
};

usbg_config_strs c_strs={
  "Serial gadget"
};

usbg_gadget_attrs g_attrs_Serial = {
    .bcdUSB = 0x0200,
    .bDeviceClass = 0,
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,
    .bMaxPacketSize0 = 64,
    .idVendor = 0x1d6b,
    .idProduct = 0x104,
    .bcdDevice = 0X0001
};

usbg_gadget_attrs g_attrs_Eth = {
    .bcdUSB = 0x0200,
    .bDeviceClass = 0,
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,
    .bMaxPacketSize0 = 64,
    .idVendor = 0x0525,
    .idProduct = 0xa4a2,
    .bcdDevice = 0X0001
};



usbotg::usbotg(QObject *parent) : QObject(parent)
{
    usbg_state *uStat;
    usbg_gadget *gadget_E, *gadget_S;
    usbg_config *c_rndis, *c_eem, *c_acm;
    QSerialPort* pSp = new QSerialPort;
    QString strCmdPkt;
    QStringList list;
    //usbg_udc* st_udc;
    //get function pointer from library file
    LinkingFp();

    //init config fs
    system("mount -t configfs none /sys/kernel/config");
    usbg_init_r("/sys/kernel/config", &uStat);

    //basic gadget option allocating(ethernet)
    init_EthGadget(uStat, gadget_E, c_rndis, c_eem);
    //basic gadget option allocating(Serial)
    init_SerialGadget(uStat, gadget_S, c_acm);

    //Default gadget -> serial
    set_EthGadget(false, GADGET_ETH_DEF_IP);
    set_SerGadget(true, pSp, gadget_S);


    //st_udc = *(usbg_get_udc_r(uStat, "ci_hdrc.0"));


    set_SerGadget(false, pSp, gadget_S);

    set_EthGadget(true, GADGET_ETH_DEF_IP);

    //just test code...
    //system("/bin/dd if=/boot0/uImage of=/dev/mmcblk3 bs=512 seek=2560");
    //execl("/bin/dd", "/bin/dd", "if=/boot0/uImage","of=/dev/mmcblk3", "bs=512", "seek=2560", NULL);

    qDebug("USB OTG : Ethernet ON");

}




void usbotg::LibLoading(QString LibPath, QLibrary* LibObj)
{
    LibObj->setFileName(LibPath.toStdString().c_str());

    if(LibObj->isLoaded() == true){
        LibObj->unload();
    }
    if(LibObj->load() == false){
        qDebug("loading fail...\n");
    }
    else{
        qDebug("Load Success!!");
    }
}



/**
 * @brief 라이브러리(*.so)에 있는 함수의 포인터들을 얻어온다.
 *
 */
void usbotg::LinkingFp()
{
    LibLoading("/usr/lib/libusbgx.so", &libusb);

    usbg_init_r             = (_usbg_init)libusb.resolve("usbg_init");
    usbg_create_gadget_r    = (_usbg_create_gadget)libusb.resolve("usbg_create_gadget");
    usbg_create_function_r  = (_usbg_create_function)libusb.resolve("usbg_create_function");
    usbg_error_name_r       = (_usbg_error_name*)libusb.resolve("usbg_error_name");
    usbg_strerror_r         = (_usbg_strerror*)libusb.resolve("usbg_strerror");
    usbg_cleanup_r          = (_usbg_cleanup)libusb.resolve("usbg_cleanup");
    usbg_free_gadget_r      = (_usbg_free_gadget)libusb.resolve("usbg_free_gadget");
    usbg_create_config_r     = (_usbg_create_config)libusb.resolve("usbg_create_config");
    usbg_add_config_function_r=(_usbg_add_config_function)libusb.resolve("usbg_add_config_function");
    usbg_write_string_r     = (_usbg_write_string)libusb.resolve("usbg_write_string");
    usbg_write_buf_r        = (_usbg_write_buf)libusb.resolve("usbg_write_buf");
    usbg_free_binding_r     = (_usbg_free_binding)libusb.resolve("usbg_free_binding");
    usbg_rm_binding_r       = (_usbg_rm_binding)libusb.resolve("usbg_rm_binding");
    usbg_set_net_dev_addr_r = (_usbg_set_net_dev_addr)libusb.resolve("usbg_set_net_dev_addr");
    usbg_set_net_host_addr_r= (_usbg_set_net_host_addr)libusb.resolve("usbg_set_net_host_addr");
    usbg_get_udc_r          = (_usbg_get_udc*)libusb.resolve("usbg_get_udc");
}

/**
 * @brief 가젯 생성 중 에러 발생시 메세지 출력.
 *
 * @param err 에러코드
 */
void usbotg::priErr(usbg_error err)
{
    if(err == USBG_ERROR_EXIST)
    {
        qDebug("Gadget already Exist!");
    }
    else if(err == USBG_ERROR_INVALID_PARAM)
    {
        qDebug("Gadget already Exist!");
    }
}
//ip related code was sourced from below link!
//https://stackoverflow.com/questions/6652384/how-to-set-the-ip-address-from-c-in-linux
/**
 * @brief 이더넷 인터페이스의 ip를 설정한다.
 *
 * @param iface_name 이더넷 인터페이스의 이름
 * @param ip_addr 설정할 ip 주소
 * @return int
 */
int usbotg::set_ip(char *iface_name, const char *ip_addr)
{
    struct ifreq ifr;
    struct sockaddr_in sin;
    int sockfd;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd == -1){
        fprintf(stderr, "Could not get socket.\n");
        return -1;
    }
    sin.sin_family = AF_INET;

    // Convert IP from numbers and dots to binary notation
    inet_aton(ip_addr,&sin.sin_addr);

    /* get interface name */
    strncpy(ifr.ifr_name, iface_name, IFNAMSIZ);

    memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));

    // Set interface address
    if (ioctl(sockfd, SIOCSIFADDR, &ifr) < 0) {
        fprintf(stderr, "Cannot set IP address. ");
        perror(ifr.ifr_name);
        return -1;
    }
    return 0;
}

/**
 * @brief 시리얼 가젯을 configfs 상에 설정한다.
 *
 * @param uStat
 * @param gadget    가젯 자체에 대한 정보
 * @param c         설정 값
 * @return int
 */
int usbotg::init_SerialGadget(usbg_state *uStat, usbg_gadget *gadget, usbg_config *c)
{
    usbg_error usbg_ret;
    usbg_ret = static_cast<usbg_error> (usbg_create_gadget_r(uStat, GADGET_SERIAL, &g_attrs_Serial, &g_strs, &gadget));
    if(usbg_ret != 0)       goto err;

    usbg_ret = static_cast<usbg_error> (usbg_create_function_r(gadget, F_ACM, "GS0", NULL, &f_acm0));
    if(usbg_ret != 0)       goto err;

    usbg_ret = static_cast<usbg_error> (usbg_create_config_r(gadget, 1, NULL, NULL, &c_strs, &c) );
    if(usbg_ret != 0)       goto err;

    usbg_ret = static_cast<usbg_error> (usbg_add_config_function_r(c, "acm.GS0", f_acm0));
    if(usbg_ret != 0)       goto err;

    err:
        priErr(usbg_ret);

    return usbg_ret;
}


/**
 * @brief get current gadget setting and set
 *
 * @param uStat     가젯의 상태가 저장되는 변수(경로)
 * @param gadget    가젯의 정보가 담길 변수
 * @param c1        가젯의 설정 정보가 저장되는 변수
 * @param c2        가젯의 설정 정보가 저장되는 변수
 * @return int
 */
int usbotg::init_EthGadget(usbg_state *uStat, usbg_gadget *gadget, usbg_config *c1, usbg_config *c2)
{
    usbg_error usbg_ret;

    //creating
    usbg_ret = static_cast<usbg_error> (usbg_create_gadget_r(uStat, GADGET_ETH, &g_attrs_Eth, &g_strs, &gadget));
    if(usbg_ret != 0)       goto err;

    //creating function usb0
    usbg_ret = static_cast<usbg_error> (usbg_create_function_r(gadget, F_RNDIS, "usb0", NULL, &f_rndis0));
    if(usbg_ret != 0)       goto err;

    //creating function usb1
    usbg_ret = static_cast<usbg_error> (usbg_create_function_r(gadget, F_EEM, "usb1", NULL, &f_eem0));
    if(usbg_ret != 0)       goto err;

    //creating config1
    usbg_ret = static_cast<usbg_error> (usbg_create_config_r(gadget, 1, NULL, NULL, &c_strs, &c1));
    if(usbg_ret != 0)       goto err;

    //creating config 2
    usbg_ret = static_cast<usbg_error> (usbg_create_config_r(gadget, 2, NULL, NULL, &c_strs, &c2));
    if(usbg_ret != 0)       goto err;

    //adding config function to Config.1
    usbg_ret = static_cast<usbg_error> (usbg_add_config_function_r(c1, "rndis.usb0", f_rndis0));
    if(usbg_ret != 0)       goto err;

    //adding config function to Config.2
    usbg_ret = static_cast<usbg_error> (usbg_add_config_function_r(c2, "eem.usb1", f_eem0));
    if(usbg_ret != 0)       goto err;

    err:
        priErr(usbg_ret);

    return usbg_ret;
}


int usbotg::uart_init()
{
    struct termios options;

    uart_fd = open(usb_uart , O_RDWR | O_NOCTTY);
    fcntl(uart_fd, F_SETFL, 0);

    tcgetattr(uart_fd, &options);
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~PARODD;
    options.c_cflag |= CS8;
    options.c_cflag &= ~CRTSCTS;

    options.c_lflag &= ~(ICANON | IEXTEN | ISIG | ECHO);
    options.c_oflag &= ~OPOST;
    options.c_iflag &= ~(ICRNL | INPCK | ISTRIP | IXON | BRKINT );

    options.c_cc[VMIN] = 1;
    options.c_cc[VTIME] = 0;

    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);

    tcsetattr(uart_fd, TCSANOW, &options);

    return uart_fd;
}

/**
 * @brief 시리얼 포트를 연다.
 *
 * @param pSP
 */
void usbotg::set_SerPortOpen(QSerialPort* pSP)
{
#if 0
    pSP->setPortName("/dev/ttyGS0");
    pSP->setBaudRate(QSerialPort::Baud115200);
    pSP->setDataBits(QSerialPort::Data8);
    pSP->setParity(QSerialPort::NoParity);
    pSP->setStopBits(QSerialPort::OneStop);

    if(!pSP->open(QIODevice::ReadWrite)){
        qDebug() << "Serial Port open fail!";
    }
#endif
    uart_init();
}

/**
 * @brief 시리얼 가젯을 활성화 시킨다.
 *
 * @param flag
 * @param pSP
 */
void usbotg::set_SerGadget(bool flag, QSerialPort* pSP, usbg_gadget* ug)
{
    QString str;
    if(flag == true)
    {
        usbg_write_string_r("/sys/kernel/config/usb_gadget", GADGET_SERIAL, "UDC", "ci_hdrc.0");
        //usbg_enable_gadget_r(ug, "ci_hdrc.0");
        set_SerPortOpen(pSP);
    }
    else
    {
        str.sprintf("%s%s%s","echo "" > /sys/kernel/config/usb_gadget/", GADGET_SERIAL, "/UDC");
        system(str.toStdString().c_str());

    }
}

/**
 * @brief 이더넷 가젯을 활성화 시킨다.
 *
 * @param flag
 * @param strIP
 */
void usbotg::set_EthGadget(bool flag, QString strIP)
{
    QString str;

    if(flag == true)
    {
        usbg_write_string_r("/sys/kernel/config/usb_gadget", GADGET_ETH, "UDC", "ci_hdrc.0");
        set_ip("usb0", strIP.toStdString().c_str());
        //set_ip("usb0", GADGET_ETH_DEF_IP);
        //calling ifup binary - busybox included
        system("ifconfig usb0 up");
        //execl("/sbin/ifconfig", "/sbin/ifconfig", "usb0", "up");
    }
    else
    {
        str.sprintf("%s%s%s","echo "" > /sys/kernel/config/usb_gadget/", GADGET_ETH, "/UDC");
        system(str.toStdString().c_str());
    }
}

/**
 * @brief Get the SerData object
 *
 * @param qF
 * @return QString
 *
 * fixme : notisock 쓸 예정
 */
QString usbotg::get_SerData()
{
    QString tmp;
    char buf[256]={0};

    read(uart_fd, buf, 256);

    tmp = QString(buf);
    qDebug() << tmp;

    return tmp;
}

QStringList usbotg::parse_CmdData(QString srcData)
{
    QStringList list;
    list = srcData.split("|");

    return list;
}

QString usbotg::get_FileFromHost(QString HostIp, QString FileName)
{
    CURL *curl;
    FILE *fp;
    CURLcode res;
    QString Url = "http://"+ HostIp + FileName;
    QString LocalPath = "/boot0/"+ FileName;
    curl = curl_easy_init();

    qDebug("hostip : %s", HostIp);

    if(curl){
        fp = fopen(LocalPath.toStdString().c_str(), "wb");
    }

    return LocalPath;

}
