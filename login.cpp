#include "login.h"
#include "ui_login.h"

QTextStream out;
QString login_Cookie;
QWebEngineView *login_webView;

login::login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::login)
{
    ui->setupUi(this);

    setWindowTitle("登录");
    setFixedSize(this->width(),this->height()); //禁止拖动窗口大小
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint); //禁止最大化按钮

    /*UI初始化项目*/
    ui->frame->hide();
    ui->progressBar->hide();

    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--disable-logging"); //关闭Chrome内核的控制台信息

    /*进行SSO单点登录，并获取Cookie*/
    login_webView = new QWebEngineView(this);
    QString OA_login_url = "http://kmp.hikvision.com.cn"; //通过登入知识管理平台，同时获取JsessionID和JsessionKM（使用http明文传输，Chrome内核报不安全警告）

    login_webView->page()->profile()->cookieStore()->deleteAllCookies(); //确保安全，进入系统前清空登录页面保留的所有Cookie
    login_webView->settings()->setAttribute(QWebEngineSettings::ShowScrollBars,false); //隐藏滑动条

    connect(login_webView,SIGNAL(loadFinished(bool)),this,SLOT(login_loadfinish(bool))); //绑定网页加载完成事件
    connect(login_webView->page()->profile()->cookieStore(), &QWebEngineCookieStore::cookieAdded,this,&login::slot_cookieAdded); //绑定Cookie添加事件

    login_webView->load(OA_login_url); //加载OA登录界面

    /*在frame控件上显示网页*/
    QStackedLayout* layout = new QStackedLayout(ui->frame);
    ui->frame->setLayout(layout);
    layout->addWidget(login_webView);
}

/*处理登录页面的Cookie信息*/
void login::slot_cookieAdded(const QNetworkCookie &cookie)
{
    login_Cookie.append(cookie.name()).append(" ").append(cookie.value()).append(" ");
    //qDebug()<<"Cookie Added-->"<<cookie.domain()<<cookie.name()<<cookie.value()<<endl; //打印Cookie内的所属域名、属性名、属性值

    if(login_Cookie.contains("CASTGC",Qt::CaseSensitive))
    {
        ui->label->show();
        ui->progressBar->show();
        ui->frame->hide();

        /*加载FTP文件目录路径txt*/
        if(ui->frame->isHidden())
        {
            if(!(file.fileName()==tr("%1/List_all_url.txt").arg(QCoreApplication::applicationDirPath()))) //是否已经加载
            {
                file.setFileName(tr("%1/List_all_url.txt").arg(QCoreApplication::applicationDirPath())); //exe目录下
                out.setDevice(&file);

                if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
                    qDebug()<<"Open file failed!";
            }
        }
    }
}

/*待登录页面加载完毕后进行页面显示、Cookie解析、登录确认*/
void login::login_loadfinish(bool)
{
    ui->label->hide();
    ui->frame->show();

    //qDebug()<<"Cookie:"<<login_Cookie;

    /*从Cookie中解析CASTGC令牌*/
    if(login_Cookie.contains("CASTGC",Qt::CaseSensitive))
        accept(); //登录确认，释放
}

void login::closeEvent(QCloseEvent *event)
{
    emit login_close_signal();
    event->accept();
}

login::~login()
{
    delete ui;
}
