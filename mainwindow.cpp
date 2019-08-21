#include "login.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setAttribute(Qt::WA_DeleteOnClose,true); //关闭窗口时销毁占用资源

    setWindowTitle("FQA_DF");
    setFixedSize(this->width(),this->height()); //禁止拖动窗口大小
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint); //禁止最大化按钮

    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--disable-logging"); //关闭Chrome内核的控制台信息

    login *l = new login(this);
    connect(l,SIGNAL(login_close_signal()),this,SLOT(close()));

    /*阻塞主窗体进程，等待登录确认*/
    if(l->exec() == QDialog::Accepted)
        qDebug()<<"login_accepted!";
}

/*Http状态码检测函数*/
bool MainWindow::http_stat(QNetworkReply* reply)
{
    /*获取http状态码*/
    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if(statusCode.isValid())
        qDebug() << "status code=" << statusCode.toInt();

    QVariant reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    if(reason.isValid())
        qDebug() << "reason=" << reason.toString();

    QNetworkReply::NetworkError err = reply->error();
    if(err != QNetworkReply::NoError)
    {
        qDebug() << "Failed: " << reply->errorString();
        return false;
    }
    else
        return true;
}

/*Everything IPC错误码*/
QString MainWindow::getSearchErrorString()
{
    QString str;
    DWORD error = Everything_GetLastError();

    if(error == EVERYTHING_ERROR_MEMORY)
        str = "未能为搜索查询分配内存！";

    if(error == EVERYTHING_ERROR_IPC)
        str = "IPC不可用！";

    if(error == EVERYTHING_ERROR_REGISTERCLASSEX)
        str = "未能注册搜索查询窗口类！";

    if(error == EVERYTHING_ERROR_CREATEWINDOW)
        str = "创建搜索查询窗口失败！";

    if(error == EVERYTHING_ERROR_CREATETHREAD)
        str = "创建搜索查询线程失败！";

    if(error == EVERYTHING_ERROR_INVALIDINDEX)
        str = "无效索引。索引必须大于或等于0，小于可见结果的数目！";

    if(error == EVERYTHING_ERROR_INVALIDCALL)
        str = "无效的呼叫！";

    return str;
}

void MainWindow::QIP_post(QString keyword)
{
    QNetworkRequest request; //请求
    QNetworkAccessManager *access = new QNetworkAccessManager(this); //接入

    /*绑定请求完成事件*/
    QMetaObject::Connection QIP_connect = QObject::connect(access, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished_QIP_post(QNetworkReply*)));
    Q_ASSERT(QIP_connect);

    request.setUrl(QUrl("http://pinfointernal.hikvision.com/app/searchData")); //产品信息管理平台
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded"); //报文编码格式

    /*Post请求*/
    QByteArray frame;
    frame.append(tr("page=1&rows=10&kw=%1&tp=1").arg(keyword));
    access->post(request,frame);
}

void MainWindow::requestFinished_QIP_post(QNetworkReply* reply)
{
    if(http_stat(reply))
    {
        ui->textBrowser->clear();

        QString short_path,pub_title;

        QString result = reply->readAll();

        result.remove(QRegExp("[\"}]")); //利用正则表达式修正字符集

        QStringList result_split_1 = result.split('{');
        QStringList result_split_2;

        for(int i=0;i<result_split_1.count();++i){
            result_split_2 = result_split_1.at(i).split(',');
            for(int j=0;j<result_split_2.count();++j)
            {
                //qDebug()<<result_split_2.at(j);

                result = result_split_2.at(j).trimmed();

                if(result.startsWith("short_path"))
                {
                    short_path = result.remove(0,26);

                    short_path = QUrl::toPercentEncoding(short_path,"/"); //路径URLEncode

                    qDebug()<<short_path;
                }

                if(result.startsWith("pub_title"))
                {
                    pub_title = result.remove(0,10);

                    qDebug()<<pub_title;
                }

                if(short_path!="" && pub_title!="")
                {
                    //qDebug()<<tr("<a href = http://pinfo.hikvision.com%1>%2</a>").arg(short_path_encoded).arg(pub_title);
                    ui->textBrowser->append(tr("<a href = http://pinfo.hikvision.com%1>->%2</a>").arg(short_path).arg(pub_title));
                    ui->textBrowser->append("");

                    short_path = pub_title = "";
                }
            }
        }
    }
}

QList<QFileInfo> MainWindow::Search(QString str, bool reg)
{
    QMutex mutex;
    mutex.lock();

    QList<QFileInfo> files;
    Everything_CleanUp();

    std::wstring wlpstr = str.toStdWString();
    LPCWSTR lpcwStr = wlpstr.c_str();

    if(reg)
        Everything_SetRegex(TRUE);

    Everything_SetSearch(lpcwStr);
    Everything_SetRequestFlags(EVERYTHING_REQUEST_FILE_NAME | EVERYTHING_REQUEST_PATH);

    if(!Everything_Query(TRUE))
        qDebug()<<getSearchErrorString();
    else
    {
        int results = static_cast<int>(Everything_GetNumResults());
        for (int var = 0; var < results; ++var){
            QString name = QString::fromStdWString(Everything_GetResultFileName(static_cast<DWORD>(var)));
            QString path = QDir::toNativeSeparators(QString::fromStdWString(Everything_GetResultPath(static_cast<DWORD>(var))));
            QFileInfo f(path+QDir::separator()+name);
            files.append(f);
        }
    }

    Everything_Reset();
    mutex.unlock();

    return files;
}

void MainWindow::on_pushButton_clicked()
{
    QString keyword = ui->lineEdit->text();

    if(ui->checkBox->isChecked())
        QIP_post(keyword);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
}

MainWindow::~MainWindow()
{
    delete ui;
}
