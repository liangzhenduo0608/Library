#include "book_return_dialog.h"
#include "ui_book_return_dialog.h"

book_return_Dialog::book_return_Dialog(QWidget *parent):
QDialog(parent),ui(new Ui::book_return_Dialog){
    ui->setupUi(this);

    QStandardItemModel* booklistModel=new QStandardItemModel(0,5,this);
    booklistModel->insertRow(0);
    booklistModel->setData(booklistModel->index(0,0), tr("图书编号"));
    booklistModel->setData(booklistModel->index(0,1), tr("ISBN"));
    booklistModel->setData(booklistModel->index(0,2), tr("书目"));
    booklistModel->setData(booklistModel->index(0,3), tr("借阅时间"));
    booklistModel->setData(booklistModel->index(0,4), tr("归还时间"));

    ui->borrowedbookview->setModel(booklistModel);
}

book_return_Dialog::~book_return_Dialog() {
    delete ui;
}

void book_return_Dialog::on_search_Button_clicked()
{
    if (ui->stuid_label->text() == "") {
        QMessageBox::warning(this, tr("ERROR"), tr("请输入学号！"));
        return;
    }

    QString stuid = ui->stuid_label->text();
    last_stuid = stuid;

    book_return_Dialog::load_book_list(last_stuid);
}

void book_return_Dialog::action_book_return(QString id) {
    QSqlQuery query_item("SELECT qlms_record.uuid, qlms_book_item.status, qlms_record.stuid, qlms_book_item.isbn FROM qlms_book_item LEFT JOIN qlms_record ON (qlms_record.status = 0 AND qlms_record.id = qlms_book_item.id) WHERE qlms_book_item.id = '"+ id +"'");

    if (!query_item.next()) {
        QMessageBox::warning(this, tr("ERROR"), tr("未检索到该记录！"));
        return;
    }

    if (query_item.value(1).toInt() == 1) {
        QMessageBox::warning(this, tr("ERROR"), tr("该册图书已在馆！"));
        return;
    }

    if (query_item.value(1).toInt() == -1) {
        QMessageBox::warning(this, tr("ERROR"), tr("该册图书正在进行修补！"));
        return;
    }

    QMessageBox box;
    box.setWindowTitle("CHOOSE");
    box.setText(tr("请选择要对该书进行的操作："));
    box.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    box.setButtonText(QMessageBox::Save,QString("归还"));
    box.setButtonText(QMessageBox::Discard,QString("淘汰"));
    box.setButtonText(QMessageBox::Cancel,QString("修补"));
    int msg_ret = box.exec();
    if (msg_ret == QMessageBox::Save) {
        QSqlQuery(tr("UPDATE qlms_record SET status = 1, time_return = NOW() WHERE uuid = %1").arg(query_item.value(0).toInt()));
        QSqlQuery(tr("UPDATE qlms_user SET num_borrowed = num_borrowed - 1 WHERE stuid = %1").arg(query_item.value(2).toString()));
        QSqlQuery(tr("UPDATE qlms_book_item SET status = 1 WHERE id = '%1'").arg(id));

        QMessageBox::information(this, tr("SUCCESS"), tr("操作成功，图书已经完成单册归还"));
    }
    else if (msg_ret == QMessageBox::Cancel) {
        QSqlQuery(tr("UPDATE qlms_record SET status = 1, time_return = NOW() WHERE uuid = %1").arg(query_item.value(0).toInt()));
        QSqlQuery(tr("UPDATE qlms_user SET num_borrowed = num_borrowed - 1 WHERE stuid = %1").arg(query_item.value(2).toString()));
        QSqlQuery(tr("UPDATE qlms_book_item SET status = -1 WHERE id = '%1'").arg(id));

        QMessageBox::information(this, tr("SUCCESS"), tr("操作成功，图书正在进行修补"));
    }
    else if (msg_ret == QMessageBox::Discard) {
        QSqlQuery(tr("UPDATE qlms_record SET status = 1, time_return = NOW() WHERE uuid = %1").arg(query_item.value(0).toInt()));
        QSqlQuery(tr("UPDATE qlms_user SET num_borrowed = num_borrowed - 1 WHERE stuid = '%1'").arg(query_item.value(2).toString()));
        QSqlQuery(tr("UPDATE qlms_book SET num_total = num_total - 1 WHERE isbn = '%1'").arg(query_item.value(3).toString()));
        QSqlQuery(tr("DELETE FROM qlms_book_item WHERE id = '%1'").arg(id));

        QMessageBox::information(this, tr("SUCCESS"), tr("操作成功，图书已淘汰"));
    }
    book_return_Dialog::load_book_list(last_stuid);
}

void book_return_Dialog::on_borrowedbookview_clicked(const QModelIndex &index)
{
    if (index.row() <1 ) return;

    book_return_Dialog::action_book_return(book_id_list[index.row()]);
}

void book_return_Dialog::on_return_Button_clicked()
{
    if (ui->bookId_label->text() == "") {
        QMessageBox::warning(this, tr("出错啦"), tr("您还没有填写要归还的图书编号"));
        return;
    }

    book_return_Dialog::action_book_return(ui->bookId_label->text());
}

void book_return_Dialog::load_book_list(QString stuid) {

    QStandardItemModel* booklistModel=new QStandardItemModel(0,5,this);
    booklistModel->insertRow(0);
    booklistModel->setData(booklistModel->index(0,0), tr("图书编号"));
    booklistModel->setData(booklistModel->index(0,1), tr("ISBN"));
    booklistModel->setData(booklistModel->index(0,2), tr("书目"));
    booklistModel->setData(booklistModel->index(0,3), tr("借阅时间"));
    booklistModel->setData(booklistModel->index(0,4), tr("归还时间"));
    ui->borrowedbookview->setModel(booklistModel);

    QSqlQuery query_user("SELECT stuid, name, department, num_borrowed, num_limit FROM qlms_user WHERE stuid LIKE '" + stuid + "%'");

    if (!query_user.next()) {
        QMessageBox::warning(this, tr("ERROR"), tr("该用户不存在！"));
        return;
    }

    ui->stuid_label->setText(query_user.value(0).toString());
    QSqlQuery query_record("SELECT qlms_record.id, qlms_book.isbn, qlms_book.title, qlms_record.time_borrow, qlms_record.time_return, qlms_record.status FROM qlms_record LEFT JOIN qlms_book_item ON qlms_book_item.id = qlms_record.id LEFT JOIN qlms_book ON qlms_book.isbn = qlms_book_item.isbn WHERE qlms_record.stuid = "+ stuid +" ORDER BY qlms_record.status, qlms_record.uuid DESC");

    for(int i=1;query_record.next();i++) {
        booklistModel->insertRow(i);
        book_id_list[i] = query_record.value(0).toString();
        booklistModel->setData(booklistModel->index(i,0), query_record.value(0).toString());
        booklistModel->setData(booklistModel->index(i,1), query_record.value(1).toString());
        booklistModel->setData(booklistModel->index(i,2), query_record.value(2).toString());
        booklistModel->setData(booklistModel->index(i,3), query_record.value(3).toString());
        booklistModel->setData(booklistModel->index(i,4), query_record.value(5).toInt() == 0 ? tr("未归还") :query_record.value(4).toString());
    }
}
