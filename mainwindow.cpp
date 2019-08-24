#include "login.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setAttribute(Qt::WA_DeleteOnClose,true); //�رմ���ʱ����ռ����Դ

    setWindowTitle("FQA_DF");
    setFixedSize(this->width(),this->height()); //��ֹ�϶����ڴ�С
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint); //��ֹ��󻯰�ť

    login *l = new login(this);
    connect(l,SIGNAL(login_close_signal()),this,SLOT(close()));

    /*������������̣��ȴ���¼ȷ��*/
    if(l->exec() == QDialog::Accepted)
        qDebug()<<"login_accepted!";

    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--disable-logging"); //�ر�Chrome�ں˵Ŀ���̨��Ϣ

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

    ui->textBrowser->append("<font size='10' color='white'>��Ʒ��Ϣƽ̨</font>");
    ui->textBrowser_2->append("<font size='8' color='white'>֪ʶ����ƽ̨</font>");
    ui->textBrowser_3->append("<font size='10' color='white'>FTP</font>");

    connect(ui->textBrowser_2,SIGNAL(anchorClicked(const QUrl)),this,SLOT(anchorClickedSlot(const QUrl)));
}

/*Http״̬���⺯��*/
bool MainWindow::http_stat(QNetworkReply* reply)
{
    /*��ȡhttp״̬��*/
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

/*Everything IPC������*/
QString MainWindow::getSearchErrorString()
{
    QString str;
    DWORD error = Everything_GetLastError();

    if(error == EVERYTHING_ERROR_MEMORY)
        str = "δ��Ϊ������ѯ�����ڴ棡";

    if(error == EVERYTHING_ERROR_IPC)
        str = "IPC�����ã�";

    if(error == EVERYTHING_ERROR_REGISTERCLASSEX)
        str = "δ��ע��������ѯ�����࣡";

    if(error == EVERYTHING_ERROR_CREATEWINDOW)
        str = "����������ѯ����ʧ�ܣ�";

    if(error == EVERYTHING_ERROR_CREATETHREAD)
        str = "����������ѯ�߳�ʧ�ܣ�";

    if(error == EVERYTHING_ERROR_INVALIDINDEX)
        str = "��Ч����������������ڻ����0��С�ڿɼ��������Ŀ��";

    if(error == EVERYTHING_ERROR_INVALIDCALL)
        str = "��Ч�ĺ��У�";

    return str;
}

/*��װpost�ӿ�*/
void MainWindow::post(int post_id,QUrl url,QString content_type,QString rows,QString keyword)
{
    QNetworkRequest request; //����
    QNetworkAccessManager *access = new QNetworkAccessManager(this); //����

    /*����������¼�(QIP_rows��0x01��QIP_post��0x02)*/
    QMetaObject::Connection QIP_connect;
    if(post_id == 0x01)
        QIP_connect = QObject::connect(access, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished_QIP_rows_post(QNetworkReply*)));
    else if(post_id == 0x02)
        QIP_connect = QObject::connect(access, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished_QIP_result_post(QNetworkReply*)));

    Q_ASSERT(QIP_connect);

    /*cookie����*/
    QVariant cookie;
    QList<QNetworkCookie> *listcookie=new QList<QNetworkCookie>();

    listcookie->push_back(QNetworkCookie("jsession_km",login_Cookie.section(' ',5,5).toUtf8()));
    cookie.setValue(*listcookie);

    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,content_type); //���ı����ʽ
    request.setHeader(QNetworkRequest::CookieHeader,cookie); //����Cookie��Ϣ

    /*Post����*/
    QByteArray frame;
    frame.append(tr("page=1&rows=%1&kw=%2&tp=1").arg(rows).arg(keyword));
    access->post(request,frame);
}

/*��װget�ӿ�*/
void MainWindow::get(int get_id,QUrl url,QString content_type)
{
    QNetworkRequest request; //����
    QNetworkAccessManager *access = new QNetworkAccessManager(this); //����

    /*����������¼�(KMP_rows��0x01��KMP_get��0x02��KMP)*/
    QMetaObject::Connection GET_connect;
    if(get_id == 0x01)
        GET_connect = QObject::connect(access, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished_KMP_result_get(QNetworkReply*)));
    else if(get_id == 0x02)
        GET_connect = QObject::connect(access, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished_KMP_htmlNums_get(QNetworkReply*)));
    else if(get_id == 0x03)
        GET_connect = QObject::connect(access, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished_KMP_docxNums_get(QNetworkReply*)));
    else if(get_id == 0x04)
        GET_connect = QObject::connect(access, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished_KMP_download_get(QNetworkReply*)));

    Q_ASSERT(GET_connect);

    /*cookie����*/
    QVariant cookie;
    QList<QNetworkCookie> *listcookie=new QList<QNetworkCookie>();

    listcookie->push_back(QNetworkCookie("jsession_km",login_Cookie.section(' ',3,3).toUtf8()));
    listcookie->push_back(QNetworkCookie("SESSION",KMP_Cookie.section(' ',3,3).toUtf8()));
    cookie.setValue(*listcookie);

    request.setUrl(url.toString());
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute,true); //���ض���
    request.setHeader(QNetworkRequest::ContentTypeHeader,content_type); //���ı����ʽ
    request.setHeader(QNetworkRequest::CookieHeader,cookie); //����Cookie��Ϣ

    access->get(request);//Get����
}

void MainWindow::requestFinished_QIP_rows_post(QNetworkReply* reply)
{
    if(http_stat(reply))
    {
        QString result = reply->readLine();

        result.remove(QRegExp("[\"{}\\]]")); //����������ʽ�����ַ���

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
}

void MainWindow::requestFinished_QIP_result_post(QNetworkReply* reply)
{
    if(http_stat(reply))
    {
        ui->textBrowser->clear();
        ui->textBrowser->append("<font size='10' color='white'>��Ʒ��Ϣƽ̨</font>");

        QString short_path,pub_title;

        QString result = reply->readAll();

        result.remove(QRegExp("[\"{}\\]]")); //����������ʽ�����ַ���

        QStringList result_split = result.split(',');

        for(int i=0;i<result_split.count();++i){
            //qDebug()<<result_split.at(i);

            result = result_split.at(i);

            if(result.startsWith("short_path"))
                short_path = QUrl::toPercentEncoding(result.remove(0,26),"/"); //·��URLEncode

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

        ui->textBrowser->moveCursor(QTextCursor::Start);
    }
}

void MainWindow::requestFinished_KMP_result_get(QNetworkReply* reply)
{
    if(http_stat(reply))
    {
        ui->textBrowser_2->clear();
        ui->textBrowser_2->append("<font size='6' color='white'>֪ʶ����ƽ̨</font>");

        int count = 0;

        QString href,KMP_target;

        QString result = reply->readAll();

        result.remove(QRegExp("[\"\r\n]")).remove("</a>"); //����������ʽ�����ַ���

        QStringList result_split = result.split('<');

        for(int i=0;i<result_split.count();++i){
            //qDebug()<<result_split.at(i);

            if(result_split.at(i).startsWith("a href=/document/"))
            {
                href = result_split.at(i).section(' ',1,1).remove(0,5);
                KMP_target = result_split.at(i).section(' ',3,3).remove(0,13);
            }

            if(!href.isEmpty() && !KMP_target.isEmpty())
            {
                //qDebug()<<tr("<a href = http://kmp.hikvision.com.cn%1>>%2</a>").arg(href).arg(target);
                ui->textBrowser_2->append(tr("<a href = http://kmp.hikvision.com.cn%1>%2</a>").arg(href).arg(KMP_target));
                ui->textBrowser_2->append("");

                if(count==0)
                    anchorClickedSlot(QUrl(tr("http://kmp.hikvision.com.cn%1").arg(href)));

                ++count;
                href = KMP_target = "";
            }
        }

        ui->textBrowser->moveCursor(QTextCursor::Start);
    }
}

void MainWindow::requestFinished_KMP_htmlNums_get(QNetworkReply *reply)
{
    if(http_stat(reply))
    {
        QString result = reply->readAll();

        result.remove(QRegExp("[\"\r\n]")).remove("</a>"); //����������ʽ�����ַ���

        QStringList result_split = result.split('<');

        for(int i=0;i<result_split.count();++i){
            //qDebug()<<result_split.at(i);

            if(result_split.at(i).startsWith("a id=titleDoc"))
                KMP_docxTitle = result_split.at(i).section(';',4,4).remove(QRegExp("\\s")); //�ļ��������ո�

            if(result_split.at(i).startsWith("a href=http://preview.hikvision.com.cn/fileserver/"))
                KMP_htmlNums = result_split.at(i).section(' ',1,1).remove(0,5);

            if(!KMP_docxTitle.isEmpty() && !KMP_htmlNums.isEmpty())
                break;
        }

        if(!KMP_htmlNums.isEmpty())
            KMP_fileserver_preview(KMP_htmlNums);
    }
}

void MainWindow::requestFinished_KMP_docxNums_get(QNetworkReply *reply)
{
    if(http_stat(reply))
    {
        QString result = reply->readAll();

        result.remove(QRegExp("[\"\r\n]")).remove("</a>"); //����������ʽ�����ַ���

        QStringList result_split = result.split('<');

        for(int i=0;i<result_split.count();++i){
            //qDebug()<<result_split.at(i);

            if(result_split.at(i).startsWith("a class"))
            {
                KMP_docxNums = result_split.at(i).section(' ',5,5).remove(0,6);

                break;
            }
        }

        if(!KMP_docxNums.isEmpty())
            get(0x04,QUrl(tr("http://preview.hikvision.com.cn/fileserver/%1").arg(KMP_docxNums)),"text/html");
    }
}

void MainWindow::requestFinished_KMP_download_get(QNetworkReply *reply)
{
    if(http_stat(reply))
    {
        connect(reply,SIGNAL(downloadProgress(qint64,qint64)),this,SLOT(updateDataReadProgress(qint64,qint64)));

        qDebug()<<KMP_docxTitle;

        file=new QFile(tr("%1/files/%2.docx").arg(QCoreApplication::applicationDirPath().arg(KMP_docxTitle)));
        file->open(QIODevice::WriteOnly);

        file->write(reply->readAll());
    }
}

void MainWindow::anchorClickedSlot(const QUrl url)
{
    get(0x02,url,"text/html");
}

void MainWindow::KMP_fileserver_preview(const QUrl url)
{
    connect(KMP_webView,SIGNAL(loadFinished(bool)),this,SLOT(KMP_loadfinish(bool))); //����ҳ��������¼�
    connect(KMP_webView->page()->profile()->cookieStore(), &QWebEngineCookieStore::cookieAdded,this,&MainWindow::slot_cookieAdded); //��Cookie����¼�

    KMP_webView->load(url); //����KMPԤ������
    KMP_webView->setZoomFactor(2); //�Ŵ�ҳ��

    /*��frame�ؼ�����ʾ��ҳ*/
    ui->frame->setLayout(layout);
    layout->addWidget(KMP_webView);
}

void MainWindow::KMP_loadfinish(bool)
{
    get(0x03,KMP_htmlNums,"text/html");
}

void MainWindow::ftp_research(QString keyword)
{
    int count = 0;

    while(!out.atEnd())
    {
        if(flag_ui_flush==0)
        {
            qDebug()<<"enter_here";

            ++count;

            QString strReadLine = out.readLine();
            QByteArray byte_urlEncoded = strReadLine.toUtf8().toPercentEncoding();

            if(strReadLine.contains(keyword))
            {
                flag_ui_flush = 1;
                emit ftp_signal(strReadLine,byte_urlEncoded,count);
            }
        }
    }
}

void MainWindow::ftp_research_Slot(QString strReadLine,QString byte_urlEncoded,int count)
{
    ui->textBrowser_3->append(tr("<a href=%1>%2</a>").arg(byte_urlEncoded).arg(strReadLine));
    ui->progressBar_2->setValue(count);

    flag_ui_flush = 0;
}

/*����Ԥ��ҳ���Cookie��Ϣ*/
void MainWindow::slot_cookieAdded(const QNetworkCookie &cookie)
{
    KMP_Cookie.append(cookie.name()).append(" ").append(cookie.value()).append(" ");
    //qDebug()<<"Cookie Added-->"<<cookie.domain()<<cookie.name()<<cookie.value()<<endl; //��ӡCookie�ڵ�����������������������ֵ
}

void MainWindow::updateDataReadProgress(qint64 bytesRead, qint64 totalBytes)
{
    ui->progressBar->setMaximum(static_cast<int>(totalBytes));
    ui->progressBar->setValue(static_cast<int>(bytesRead));
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
    if(ui->checkBox->isChecked())
        post(0x01,QUrl("http://pinfointernal.hikvision.com/app/searchData"),"application/x-www-form-urlencoded","10",ui->lineEdit->text());

    if(ui->checkBox_2->isChecked())
        get(0x01,QUrl(tr("http://kmp.hikvision.com.cn/search-title-modelType?search=%1").arg(ui->lineEdit->text())),"text/html");

    if(ui->checkBox_3->isChecked())
    {
        connect(this,SIGNAL(ftp_signal(QString,QString,int)),this,SLOT(ftp_research_Slot(QString,QString,int)));

        QFuture<void> fut = QtConcurrent::run(this,&MainWindow::ftp_research,ui->lineEdit->text());

        while(!fut.isFinished())
            QApplication::processEvents();
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
