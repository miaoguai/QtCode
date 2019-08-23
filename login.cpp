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

    setWindowTitle("��¼");
    setFixedSize(this->width(),this->height()); //��ֹ�϶����ڴ�С
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint); //��ֹ��󻯰�ť

    /*UI��ʼ����Ŀ*/
    ui->frame->hide();
    ui->progressBar->hide();

    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--disable-logging"); //�ر�Chrome�ں˵Ŀ���̨��Ϣ

    /*����SSO�����¼������ȡCookie*/
    login_webView = new QWebEngineView(this);
    QString OA_login_url = "http://kmp.hikvision.com.cn"; //ͨ������֪ʶ����ƽ̨��ͬʱ��ȡJsessionID��JsessionKM��ʹ��http���Ĵ��䣬Chrome�ں˱�����ȫ���棩

    login_webView->page()->profile()->cookieStore()->deleteAllCookies(); //ȷ����ȫ������ϵͳǰ��յ�¼ҳ�汣��������Cookie
    login_webView->settings()->setAttribute(QWebEngineSettings::ShowScrollBars,false); //���ػ�����

    connect(login_webView,SIGNAL(loadFinished(bool)),this,SLOT(login_loadfinish(bool))); //����ҳ��������¼�
    connect(login_webView->page()->profile()->cookieStore(), &QWebEngineCookieStore::cookieAdded,this,&login::slot_cookieAdded); //��Cookie����¼�

    login_webView->load(OA_login_url); //����OA��¼����

    /*��frame�ؼ�����ʾ��ҳ*/
    QStackedLayout* layout = new QStackedLayout(ui->frame);
    ui->frame->setLayout(layout);
    layout->addWidget(login_webView);
}

/*�����¼ҳ���Cookie��Ϣ*/
void login::slot_cookieAdded(const QNetworkCookie &cookie)
{
    login_Cookie.append(cookie.name()).append(" ").append(cookie.value()).append(" ");
    //qDebug()<<"Cookie Added-->"<<cookie.domain()<<cookie.name()<<cookie.value()<<endl; //��ӡCookie�ڵ�����������������������ֵ

    if(login_Cookie.contains("CASTGC",Qt::CaseSensitive))
    {
        ui->label->show();
        ui->progressBar->show();
        ui->frame->hide();

        /*����FTP�ļ�Ŀ¼·��txt*/
        if(ui->frame->isHidden())
        {
            if(!(file.fileName()==tr("%1/List_all_url.txt").arg(QCoreApplication::applicationDirPath()))) //�Ƿ��Ѿ�����
            {
                file.setFileName(tr("%1/List_all_url.txt").arg(QCoreApplication::applicationDirPath())); //exeĿ¼��
                out.setDevice(&file);

                if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
                    qDebug()<<"Open file failed!";
            }
        }
    }
}

/*����¼ҳ�������Ϻ����ҳ����ʾ��Cookie��������¼ȷ��*/
void login::login_loadfinish(bool)
{
    ui->label->hide();
    ui->frame->show();

    //qDebug()<<"Cookie:"<<login_Cookie;

    /*��Cookie�н���CASTGC����*/
    if(login_Cookie.contains("CASTGC",Qt::CaseSensitive))
        accept(); //��¼ȷ�ϣ��ͷ�
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
