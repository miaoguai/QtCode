#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include <QCloseEvent>
#include <QtWebEngineWidgets>

#pragma execution_character_set("utf-8")

extern QString login_Cookie;
extern QTextStream out;
extern QWebEngineView *login_webView;

namespace Ui {
class login;
}

class login : public QDialog
{
    Q_OBJECT

public:
    explicit login(QWidget *parent = nullptr);
    ~login();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void login_loadfinish(bool finish);
    void slot_cookieAdded(const QNetworkCookie &cookie);

signals:
    void login_close_signal();

private:
    Ui::login *ui;

    QFile file;
};

#endif // LOGIN_H
