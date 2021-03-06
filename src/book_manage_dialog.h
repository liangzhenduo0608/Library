#ifndef BOOK_MANAGE_DIALOG_H
#define BOOK_MANAGE_DIALOG_H

#include "public.h"
#include <QDialog>

namespace Ui {
class book_manage_Dialog;
}

class book_manage_Dialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit book_manage_Dialog(QWidget *parent = 0);
    ~book_manage_Dialog();
    
private slots:
    void on_single_add_clicked();

    void on_guide_booklist_clicked();

    void on_userview_clicked(const QModelIndex &index);

    void on_guide_add_single_clicked();

signals:
    void signal_load_item(QString isbn);
    void signal_show_dialog(int dialog_id);

private:
    Ui::book_manage_Dialog *ui;
    QString book_isbn[MXN];
};

#endif // BOOK_MANAGE_DIALOG_H
