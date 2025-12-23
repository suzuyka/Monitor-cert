#ifndef DATAFORMWIDGET_H
#define DATAFORMWIDGET_H

#include <QWidget>
#include <functional>
#include "database.h"

class QLineEdit;
class QTextEdit;
class QComboBox;
class QPushButton;

class DataFormWidget : public QWidget {
public:
    explicit DataFormWidget(QWidget *parent = nullptr);

    void setCertificate(const Certificate &c);
    Certificate getCertificate() const;
    void clearForm();

    std::function<void(const Certificate&)> onSave;
    std::function<void()> onCancel;

private:
    QLineEdit *nameEdit;
    QComboBox *typeCombo;
    QLineEdit *dateEdit;
    QTextEdit *descriptionEdit;
    QPushButton *saveBtn;
    QPushButton *cancelBtn;
    QPushButton *clearBtn;

    int currentId; // -1 для нового
};

#endif // DATAFORMWIDGET_H
