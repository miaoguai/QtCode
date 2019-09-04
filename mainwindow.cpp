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
        qDebug()<<"login_accepted!"<<"\n";

    for(int i=0;i<5;++i)
        ui->textBrowser->append("\n");

    for(int i=0;i<4;++i)
        ui->textBrowser_2->append("\n");

    for(int i=0;i<2;++i)
        ui->textBrowser_3->append("\n");

    ui->frame->setStyleSheet("background-image: url(:/PNG/preview.png);");

    ui->textBrowser->setAlignment(Qt::AlignCenter);
    ui->textBrowser_2->setAlignment(Qt::AlignCenter);
    ui->textBrowser_3->setAlignment(Qt::AlignCenter);

    ui->textBrowser->append("<font size='10' color='white'>产品信息平台</font>");
    ui->textBrowser_2->append("<font size='8' color='white'>知识管理平台</font>");
    ui->textBrowser_3->append("<font size='10' color='white'>FTP/Everything</font>");

    connect(ui->textBrowser_2,SIGNAL(anchorClicked(const QUrl)),this,SLOT(KMP_anchorClickedSlot(const QUrl)));
    connect(ui->textBrowser_3,SIGNAL(anchorClicked(const QUrl)),this,SLOT(ForE_anchorClickedSlot(const QUrl)));
}

/*Http状态码检测函数*/
bool MainWindow::http_stat(QNetworkReply *reply)
{
    /*获取http状态码*/
    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if(statusCode.isValid())
        qDebug()<<"status code="<<statusCode.toInt();

    QVariant reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    if(reason.isValid())
        qDebug()<<"reason="<< reason.toString();

    QNetworkReply::NetworkError err = reply->error();
    if(err!=QNetworkReply::NoError)
    {
        qDebug()<<"Failed:"<<reply->errorString();
        qDebug()<<"\n";

        return false;
    }
    else
    {
        qDebug()<<"\n";

        return true;
    }
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

/*封装get接口*/
void MainWindow::get(int get_id,QUrl url,QString content_type)
{
    QNetworkRequest request; //请求
    QNetworkAccessManager *access = new QNetworkAccessManager(); //接入

    /*绑定请求完成事件(KMP_result：0x01，KMP_htmlNums：0x02，KMP_docxNums：0x03，KMP_download：0x04)*/
    QMetaObject::Connection GET_connect;
    if(get_id==0x01)
        GET_connect = QObject::connect(access, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished_KMP_result_get(QNetworkReply*)));
    else if(get_id==0x02)
        GET_connect = QObject::connect(access, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished_KMP_htmlNums_get(QNetworkReply*)));
    else if(get_id==0x03)
        GET_connect = QObject::connect(access, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished_KMP_docxNums_get(QNetworkReply*)));
    else if(get_id==0x04)
        GET_connect = QObject::connect(access, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished_KMP_download_get(QNetworkReply*)));

    Q_ASSERT(GET_connect);

    /*cookie解析*/
    QVariant cookie;
    QList<QNetworkCookie> *listcookie=new QList<QNetworkCookie>();

    /*装载知识管理平台及其文件服务器的Cookie信息*/
    listcookie->push_back(QNetworkCookie("jsession_km",login_Cookie.section(' ',3,3).toUtf8()));
    listcookie->push_back(QNetworkCookie("SESSION",KMP_Cookie.section(' ',3,3).toUtf8()));
    cookie.setValue(*listcookie);

    request.setUrl(url);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute,true); //打开重定向
    request.setHeader(QNetworkRequest::ContentTypeHeader,content_type); //报文编码格式
    request.setHeader(QNetworkRequest::CookieHeader,cookie); //设置Cookie信息

    if(get_id==0x01 || get_id==0x02 || get_id==0x03)
        access->get(request); //Get请求
    else if(get_id==0x04)
        connect(access->get(request),SIGNAL(downloadProgress(qint64,qint64)),this,SLOT(updateDataReadProgress(qint64,qint64))); //绑定网页加载进度事件，带进度反馈的Get请求
}

/*封装post接口*/
void MainWindow::post(int post_id,QUrl url,QString content_type,QString rows,QString keyword)
{
    QNetworkRequest request; //请求
    QNetworkAccessManager *access = new QNetworkAccessManager(); //接入

    /*绑定请求完成事件(QIP_rows：0x01，QIP_post：0x02)*/
    QMetaObject::Connection QIP_connect;
    if(post_id==0x01)
        QIP_connect = QObject::connect(access, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished_QIP_rows_post(QNetworkReply*)));
    else if(post_id==0x02)
        QIP_connect = QObject::connect(access, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished_QIP_result_post(QNetworkReply*)));

    Q_ASSERT(QIP_connect);

    /*cookie解析*/
    QVariant cookie;
    QList<QNetworkCookie> *listcookie=new QList<QNetworkCookie>();

    /*装载单点登录的Cookie信息*/
    listcookie->push_back(QNetworkCookie("jsession_km",login_Cookie.section(' ',5,5).toUtf8()));
    cookie.setValue(*listcookie);

    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,content_type); //报文编码格式
    request.setHeader(QNetworkRequest::CookieHeader,cookie); //设置Cookie信息

    /*Post请求*/
    QByteArray frame;
    frame.append(tr("page=1&rows=%1&kw=%2&tp=1").arg(rows).arg(keyword));
    access->post(request,frame);
}

/*获取产品信息平台的列项信息*/
void MainWindow::requestFinished_QIP_rows_post(QNetworkReply* reply)
{
    qDebug()<<"产品信息平台列表信息请求(Post)：";
    if(http_stat(reply))
    {
        QString result = reply->readLine();

        result.remove(QRegExp("[\"{}\\]]")); //利用正则表达式修正字符集

        QStringList result_split = result.split(',');

        for(int i=0;i<result_split.count();++i){
            if(result_split.at(i).startsWith("total"))
            {
                QIP_rows = result_split.at(i);
                QIP_rows.remove(0,6);

                break;
            }
        }

        post(0x02,QUrl("http://pinfointernal.hikvision.com/app/searchData"),"application/x-www-form-urlencoded",QIP_rows,ui->lineEdit->text());
    }

    reply->deleteLater();
}

/*获取产品信息平台的检索结果*/
void MainWindow::requestFinished_QIP_result_post(QNetworkReply* reply)
{
    qDebug()<<"产品信息平台检索请求(Post)：";
    if(http_stat(reply))
    {
        QString short_path,pub_title;

        QString result = reply->readAll();

        result.remove(QRegExp("[\"{}\\]]")); //利用正则表达式修正字符集

        QStringList result_split = result.split(',');

        for(int i=0;i<result_split.count();++i){
            //qDebug()<<result_split.at(i);

            result = result_split.at(i);

            if(result.startsWith("short_path"))
                short_path = QUrl::toPercentEncoding(result.remove(0,26),"/"); //路径URLEncode

            if(result.startsWith("pub_title"))
                pub_title = result.remove(0,10);

            if(!short_path.isEmpty() && !pub_title.isEmpty())
            {
                //qDebug()<<tr("<a href = http://pinfo.hikvision.com%1>>%2</a>").arg(short_path).arg(pub_title);
                ui->textBrowser->append(tr("<a href = http://pinfo.hikvision.com%1>>%2</a>").arg(short_path).arg(pub_title));
                ui->textBrowser->append("");

                short_path = pub_title = "";
            }
        }

        if(!ui->textBrowser->toPlainText().contains('>'))
            ui->textBrowser->append("<font size='6' color='red'>无匹配结果！</font>");

        ui->textBrowser->moveCursor(QTextCursor::Start);
    }

    reply->deleteLater();
}

/*获取知识管理平台的检索结果*/
void MainWindow::requestFinished_KMP_result_get(QNetworkReply* reply)
{
    qDebug()<<"知识管理平台检索请求(Get)：";
    if(http_stat(reply))
    {
        int count = 0;

        QString href,KMP_target;

        QString result = reply->readAll();

        result.remove(QRegExp("[\"\r\n]")).remove("</a>"); //利用正则表达式修正字符集

        QStringList result_split = result.split('<');

        for(int i=0;i<result_split.count();++i){
            //qDebug()<<result_split.at(i);

            if(result_split.at(i).startsWith("a href=/document/"))
            {
                href = result_split.at(i).section(' ',1,1).remove(0,5);
                KMP_target = ">"+result_split.at(i).section('>',1);
            }

            if(!href.isEmpty() && !KMP_target.isEmpty())
            {
                //qDebug()<<tr("<a href = http://kmp.hikvision.com.cn%1>>%2</a>").arg(href).arg(KMP_target);
                ui->textBrowser_2->append(tr("<a href = http://kmp.hikvision.com.cn%1>%2</a>").arg(href).arg(KMP_target));
                ui->textBrowser_2->append("");

                if(count==0)
                    KMP_anchorClickedSlot(QUrl(tr("http://kmp.hikvision.com.cn%1").arg(href)));

                ++count;
                href = KMP_target = "";
            }
        }

        ui->textBrowser_2->moveCursor(QTextCursor::Start);
    }

    reply->deleteLater();
}

/*获取知识管理平台文件项的html编码号*/
void MainWindow::requestFinished_KMP_htmlNums_get(QNetworkReply *reply)
{
    qDebug()<<"知识管理平台html编码号请求(Get)：";
    if(http_stat(reply))
    {
        QString result = reply->readAll();

        result.remove(QRegExp("[\"\r\n]")).remove("</a>"); //利用正则表达式修正字符集

        QStringList result_split = result.split('<');

        for(int i=0;i<result_split.count();++i){
            //qDebug()<<result_split.at(i);

            if(result_split.at(i).startsWith("a id=titleDoc"))
                KMP_docxTitle = result_split.at(i).section(';',4,4).remove(QRegExp("[\\s>]")); //文件名不带空格、\、/、:、*、?、"、<、>、|

            if(result_split.at(i).startsWith("a href=http://preview.hikvision.com.cn/fileserver/"))
                KMP_htmlNums = result_split.at(i).section(' ',1,1).remove(0,5);

            if(!KMP_docxTitle.isEmpty() && !KMP_htmlNums.isEmpty())
            {
                KMP_webView->deleteLater(); //注销前次预览页面，避免重复打开

                KMP_fileserver_preview(KMP_htmlNums);

                break;
            }
        }

        if(KMP_docxTitle.isEmpty() || KMP_htmlNums.isEmpty())
        {
            QMessageBox msgBox;
            msgBox.setText("<font size='5' color='black'>非常规文档！</font>");
            msgBox.addButton(QMessageBox::Ok);
            msgBox.button(QMessageBox::Ok)->hide();
            msgBox.exec();
        }
    }

    reply->deleteLater();
}

/*获取知识管理平台文件项的docx编码号*/
void MainWindow::requestFinished_KMP_docxNums_get(QNetworkReply *reply)
{
    qDebug()<<"知识管理平台docx编码号请求(Get)：";
    if(http_stat(reply))
    {
        QString result = reply->readAll();

        result.remove(QRegExp("[\"\r\n]")).remove("</a>"); //利用正则表达式修正字符集

        QStringList result_split = result.split('<');

        for(int i=0;i<result_split.count();++i){
            //qDebug()<<result_split.at(i);

            if(result_split.at(i).startsWith("a class"))
            {
                KMP_docxNums = result_split.at(i).section(' ',5,5).remove(0,6);

                if(!KMP_docxNums.isEmpty())
                {
                    get(0x04,QUrl(tr("http://preview.hikvision.com.cn/fileserver/%1").arg(KMP_docxNums)),"text/html");

                    KMP_docxNums = "";
                }

                break;
            }
        }
    }

    reply->deleteLater();
}

/*获取知识管理平台的文件项，并保存在本地files文件夹下*/
void MainWindow::requestFinished_KMP_download_get(QNetworkReply *reply)
{
    qDebug()<<"知识管理平台文件下载请求(Get)：";
    if(http_stat(reply))
    {
        file=new QFile(tr("%1/files/%2.docx").arg(QCoreApplication::applicationDirPath()).arg(KMP_docxTitle.replace(QRegExp("[()]"),"_")));
        file->open(QIODevice::WriteOnly);

        file->write(reply->readAll());

        file->close(); //下载完成后关闭/释放文件
        KMP_docxTitle = "";
    }

    reply->deleteLater();
}

/*重写KMP结果输出控件(Qtextbrowser)的点击链接事件*/
void MainWindow::KMP_anchorClickedSlot(const QUrl url)
{
    if(flag_KMP_preview_webloadfinish==0 && flag_KMP_preview_downloadfinish==0)
    {
        ui->progressBar->setValue(0);

        get(0x02,url,"text/html");
    }
    else if(flag_KMP_preview_webloadfinish==1 || flag_KMP_preview_downloadfinish==1)
    {
        QMessageBox msgBox;
        msgBox.setText("<font size='5' color='black'>请等待加载进度完成！</font>");
        msgBox.addButton(QMessageBox::Ok);
        msgBox.button(QMessageBox::Ok)->hide();
        msgBox.exec();
    }
}

/*重写FTP/Everything结果输出控件(Qtextbrowser)的点击链接事件*/
void MainWindow::ForE_anchorClickedSlot(const QUrl url)
{
    QProcess p(nullptr);

    QTextCodec *codec = QTextCodec::codecForName("GBK");

    p.startDetached("CMD", QStringList()<<"/c"<<codec->toUnicode(url.toString().toLocal8Bit())); //脱离主窗体打开word
}

/*预览知识管理平台的文件项*/
void MainWindow::KMP_fileserver_preview(const QUrl url)
{    
    flag_KMP_preview_webloadfinish = 1;

    KMP_webView = new QWebEngineView();

    connect(KMP_webView,SIGNAL(loadFinished(bool)),this,SLOT(KMP_preview_loadfinish(bool))); //绑定网页加载完成事件
    connect(KMP_webView->page()->profile()->cookieStore(), &QWebEngineCookieStore::cookieAdded,this,&MainWindow::slot_cookieAdded); //绑定Cookie添加事件

    KMP_webView->load(url); //加载KMP预览界面
    KMP_webView->setZoomFactor(2); //放大页面

    /*在frame控件上显示网页*/
    ui->frame->setLayout(layout);
    layout->addWidget(KMP_webView);
}

/*预览页面加载完成事件*/
void MainWindow::KMP_preview_loadfinish(bool)
{   
    get(0x03,KMP_htmlNums,"text/html");

    flag_KMP_preview_webloadfinish = 0;

    KMP_htmlNums = "";
}

/*FTP检索，对txt文件下的路径信息进行多关键词检索*/
void MainWindow::ftp_research(QString keyword)
{
    int count = 0;
    flag_ftp_search_finish = 1;

    QString strReadLine;
    QByteArray byte_urlEncoded;
    QTextCodec *codec = QTextCodec::codecForName("GBK");

    while(!out.atEnd())
    {
        if(flag_ui_flush==0)
        {
            ++count;

            strReadLine = out.readLine();
            byte_urlEncoded = codec->toUnicode(strReadLine.toLocal8Bit().toPercentEncoding("/:")).toStdString().c_str();

            if(strReadLine.contains(keyword))
            {
                flag_ui_flush = 1;
                emit ftp_signal(strReadLine,byte_urlEncoded,count);
            }
        }
    }
}

/*在主窗体刷新显示控件(Qtextbrowser)，列出FTP检索结果，并更新进度条*/
void MainWindow::ftp_research_Slot(QString strReadLine,QByteArray byte_urlEncoded,int count)
{
    ui->textBrowser_3->append(tr("<a href=%1>%2</a>").arg(byte_urlEncoded.data()).arg(strReadLine));
    ui->textBrowser_3->append("");

    ui->progressBar_2->setValue(count);

    flag_ui_flush = 0;
}

/*处理预览页面的Cookie信息*/
void MainWindow::slot_cookieAdded(const QNetworkCookie &cookie)
{
    KMP_Cookie.append(cookie.name()).append(" ").append(cookie.value()).append(" ");
    //qDebug()<<"Cookie Added-->"<<cookie.domain()<<cookie.name()<<cookie.value()<<endl; //打印Cookie内的所属域名、属性名、属性值
}

/*KMP预览页面加载进度*/
void MainWindow::updateDataReadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    if(bytesSent == bytesTotal)
        flag_KMP_preview_downloadfinish = 0;
    else
        flag_KMP_preview_downloadfinish = 1;

    ui->progressBar->setMaximum(static_cast<int>(bytesTotal));
    ui->progressBar->setValue(static_cast<int>(bytesSent));
}

/*Everything文件检索*/
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

/*主窗体的'Search'按钮处理函数*/
void MainWindow::on_pushButton_clicked()
{
    if(ui->checkBox->isChecked())
    {
        ui->textBrowser->clear();
        ui->textBrowser->setAlignment(Qt::AlignLeft);
        ui->textBrowser->append("<font size='10' color='white'>产品信息平台</font>");

        post(0x01,QUrl("http://pinfointernal.hikvision.com/app/searchData"),"application/x-www-form-urlencoded","10",ui->lineEdit->text());
    }

    if(ui->checkBox_2->isChecked())
    {
        if(flag_KMP_preview_webloadfinish==0 && flag_KMP_preview_downloadfinish==0)
        {
            ui->textBrowser_2->clear();
            ui->textBrowser_2->setAlignment(Qt::AlignLeft);
            ui->textBrowser_2->append("<font size='6' color='white'>知识管理平台</font>");

            get(0x01,QUrl(tr("http://kmp.hikvision.com.cn/search-title-modelType?search=%1").arg(ui->lineEdit->text())),"text/html");
        }
        else if(flag_KMP_preview_webloadfinish==1 || flag_KMP_preview_downloadfinish==1)
        {
            QMessageBox msgBox;
            msgBox.setText("<font size='5' color='black'>请等待加载进度完成！</font>");
            msgBox.addButton(QMessageBox::Ok);
            msgBox.button(QMessageBox::Ok)->hide();
            msgBox.exec();
        }
    }

    if(ui->checkBox_3->isChecked())
    {
        if(flag_ftp_search_finish==0)
        {
            ui->checkBox_3->setEnabled(false);

            ui->textBrowser_3->clear();
            ui->textBrowser_3->setOpenLinks(true);
            ui->textBrowser_3->setAlignment(Qt::AlignLeft);
            ui->textBrowser_3->append("<font size='6' color='white'>FTP</font>");

            connect(this,SIGNAL(ftp_signal(QString,QByteArray,int)),this,SLOT(ftp_research_Slot(QString,QByteArray,int))); //绑定Qtextbrowse刷新事件

            QFuture<void> fut = QtConcurrent::run(this,&MainWindow::ftp_research,ui->lineEdit->text()); //新建并行的FTP搜索线程

            /*避免主窗体假死*/
            while(!fut.isFinished())
                QApplication::processEvents();

            out.seek(0);
            flag_ftp_search_finish = 0;

            ui->checkBox_3->setEnabled(true);

            ui->progressBar_2->setValue(List_all_url);

            ui->textBrowser_3->moveCursor(QTextCursor::Start);

            QMessageBox msgBox;
            msgBox.setText("<font size='5' color='black'>ftp搜索完成！</font>");
            msgBox.addButton(QMessageBox::Ok);
            msgBox.button(QMessageBox::Ok)->hide();
            msgBox.exec();

            qDebug()<<"ftp_search_done!";
        }
        else if(flag_ftp_search_finish==1 && !ui->checkBox->isChecked() && !ui->checkBox_2->isChecked())
        {
            QMessageBox msgBox;
            msgBox.setText("<font size='5' color='black'>请等待ftp搜索完成！</font>");
            msgBox.addButton(QMessageBox::Ok);
            msgBox.button(QMessageBox::Ok)->hide();
            msgBox.exec();
        }
    }

    if(ui->checkBox_4->isChecked())
    {
        ui->checkBox_4->setEnabled(false);

        ui->textBrowser_3->clear();
        ui->textBrowser_3->setOpenLinks(false);
        ui->textBrowser_3->setAlignment(Qt::AlignLeft);
        ui->textBrowser_3->append("<font size='6' color='white'>Everything</font>");

        QList<QFileInfo> result = Search(tr("%1/files file:docx <content:%2>").arg(QCoreApplication::applicationDirPath()).arg(ui->lineEdit->text()),false);

        for(int i=0;i<result.count();++i)
        {
            ui->textBrowser_3->append(tr("<a href=%1>%1</a>").arg(result.at(i).filePath()));
            ui->textBrowser_3->append("");
        }

        ui->checkBox_4->setEnabled(true);

        ui->textBrowser_3->moveCursor(QTextCursor::Start);
    }

    if(!ui->checkBox->isChecked() && !ui->checkBox_2->isChecked() && !ui->checkBox_3->isChecked() && !ui->checkBox_4->isChecked())
    {
        QMessageBox msgBox;
        msgBox.setText("<font size='5' color='black'>请勾选对应项后操作！</font>");
        msgBox.addButton(QMessageBox::Ok);
        msgBox.button(QMessageBox::Ok)->hide();
        msgBox.exec();
    }
}

void MainWindow::on_checkBox_3_clicked()
{
    if(ui->checkBox_3->isChecked())
        ui->checkBox_4->setEnabled(false);
    else
        ui->checkBox_4->setEnabled(true);
}

void MainWindow::on_checkBox_4_clicked()
{
    if(ui->checkBox_4->isChecked())
    {
        ui->checkBox->setChecked(false);
        ui->checkBox_2->setChecked(false);
        ui->checkBox_3->setChecked(false);

        ui->checkBox->setEnabled(false);
        ui->checkBox_2->setEnabled(false);
        ui->checkBox_3->setEnabled(false);
    }
    else
    {
        ui->checkBox->setEnabled(true);
        ui->checkBox_2->setEnabled(true);
        ui->checkBox_3->setEnabled(true);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    login_webView->deleteLater();
    KMP_webView->deleteLater();

    event->accept();
}

MainWindow::~MainWindow()
{
    delete ui;
}
