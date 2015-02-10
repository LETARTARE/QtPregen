//------------------------------------------------------------------------------
// qtest.h
//------------------------------------------------------------------------------
#ifndef _QTEST_H
#define _QTEST_H
//------------------------------------------------------------------------------
#include <QWidget>
#include <QPushButton>
//------------------------------------------------------------------------------

class Qtest : public QWidget
{
	Q_OBJECT

    public:
		Qtest ( QWidget *parent = 0, Qt::WFlags flags = 0 );
		virtual ~Qtest ();

    protected:

	protected slots:
		void press();
		void message();

    private:
        QPushButton * button;

    signals:
        void click();
};

#endif // _QTEST_H
