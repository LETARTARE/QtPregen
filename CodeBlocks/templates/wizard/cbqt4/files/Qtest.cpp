//------------------------------------------------------------------------------
// qtest.cpp
//------------------------------------------------------------------------------
#include "Qtest.h"

#include <QMessageBox>
//------------------------------------------------------------------------------
//ctor
Qtest::Qtest(QWidget *parent, Qt::WFlags flags)
	: QWidget(parent, flags)
{
    button = new QPushButton("Test", this);

   	connect( button, SIGNAL(clicked()), this, SLOT(press()) );

   	connect( this, SIGNAL(click()), this, SLOT(message()) );
}
//------------------------------------------------------------------------------
//dtor
Qtest::~Qtest()
{
}
//------------------------------------------------------------------------------
void Qtest::press()
{
    QMessageBox::information(this, "Qtest", "Slots is OK !");
    emit click();
}
//------------------------------------------------------------------------------
void Qtest::message()
{
    QMessageBox::information(this, "Qtest", "Signals is OK !");
}
//------------------------------------------------------------------------------
