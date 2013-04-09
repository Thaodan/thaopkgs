#ifndef ICONDIALOG_H
#define ICONDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QFileDialog>
#include <QDesktopServices>
#include <QPixmap>

namespace Ui {
class IconDialog;
}

class IconDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QIcon icon READ icon)
    Q_PROPERTY(QString fileName READ fileName)
    Q_PROPERTY(QString settingsDir WRITE setSettingsDir)
    
public:
    explicit IconDialog(QWidget *parent = 0);
    ~IconDialog();
    
    QIcon icon() const
    {
        return m_icon;
    }

    QString fileName() const
    {
        return m_fileName;
    }

public slots:
    void setSettingsDir(QString arg)
    {
        m_settingsDir = arg;
    }

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_helpRequested();

private:
    Ui::IconDialog *ui;
    QIcon m_icon;
    QString m_fileName;
    QString m_settingsDir;
};

#endif // ICONDIALOG_H
