#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QColorDialog>

/* Для того, чтобы можно было сохранить список окружностей, нужно научить программу сохранять каждый объект класса Circle: */
QDataStream& operator <<(QDataStream& ostream, const Circle& c)
{
    ostream << c.p.x()<< c.p.y() <<  c.r << c.color ;
    return ostream;
}

 //  Описываем, как считывать из потока каждый объект класса circle:
QDataStream& operator >> (QDataStream& istream, Circle& c)
{
    int x, y;
    istream >> x >> y >> c.r >> c.color;
    c.p.setX(x);
    c.p.setY(y);
    return istream;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //  связываем сигналы от команд меню с соответствующими слотами:
    connect(ui->actionAnnul, SIGNAL(triggered()), this, SLOT(Annul()));
    connect(ui->actionClear, SIGNAL(triggered()), this, SLOT(Clear()));
    connect(ui->actionColor, SIGNAL(triggered()), this, SLOT(showDialog()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(save()));
    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(open()));

    // инициализируем элементы данных класса MainWindow:

    drawMode = false;
    p[0] = p[1] = QPoint(0,0);
    r = 0;
    color = Qt::darkCyan;
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Обрабатываем нажатие кнопки мыши:
void MainWindow::mousePressEvent(QMouseEvent * e)
{
    if (e->button() ==Qt::LeftButton) // если нажата левая кнопка
    {
        drawMode = true; // разрешаем рисование
        p[0] = e->pos(); // запоминаем центр окружности
    }
}

// Обрабатываем движение мыши:
void MainWindow::mouseMoveEvent(QMouseEvent * e)
{
    if (drawMode) // если прижата левая кнопка
    {
        p[1] = e->pos(); // запоминаем текущую точку на окружности
        r = sqrt(pow(p[1].x()-p[0].x(),2) + pow(p[1].y()-p[0].y(),2)); // вычисляем радиус окружности
        update(); // перерисовываем окно
    }
}

// Обрабатываем отпускание кнопки мыши:
void MainWindow::mouseReleaseEvent(QMouseEvent *)
{
    drawMode = false; // запрещаем рисование
    int r = sqrt(pow(p[1].x()-p[0].x(),2) + pow(p[1].y()-p[0].y(),2)); // вычисляем текущий радиус
    DrawBuffer.append(Circle(p[0], r, color)); // добавляем окружность в список
    update(); // перерисовываем окно
}

// Описываем, что нужно нарисовать в окне
void MainWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    int size = DrawBuffer.size();
    if (size) // Если список не пуст:
    {
        // рисуем окружности, сохраненные в списке:
        QList<Circle>::const_iterator it = DrawBuffer.begin(); // итератор для прохода по списку
        do
        {
            Circle c = *it++;
            painter.setPen(c.color);
            painter.drawEllipse(c.p, c.r, c.r);
        } while (it!= DrawBuffer.end()); // рисуем окружности, пока список не закончится
    }

/* рисуем текущую окружность: окружность изменяется в зависимости от движения мыши, поэтому мы ее будем рисовать отдельно в объекте pixmap и накладывать этот объект на окно, тогда не нужно стирать предыдущий вид этой окружности */
if (drawMode) // если прижата левая кнопка мыши
    { // определяем размеры области рисования:
    int w = width();
    int h = height()-(ui->statusBar->height());

    QPixmap pixmap(w, h); // создаем область, в которой мы будем рисовать текущую окружность
    pixmap.fill(Qt::transparent); // делаем фон прозрачным
    QPainter pntPixmap(&pixmap); // создаем объект для рисования
    pntPixmap.setRenderHint(QPainter::Antialiasing); // сглаживание
    pntPixmap.setPen(color); /* устанавливаем цвет пера (который мы можем выбрать с помощью специального окна) */

    pntPixmap.drawEllipse(p[0],r,r); // рисуем текущую окружность в области pixmap
    painter.drawPixmap(0, 0, w, h, pixmap); // накладываем текущую окружность на ранее нарисованные
    }
}


// Слот для кнопки «Выбор цвета»:
void MainWindow::showDialog()
{
    color = QColorDialog::getColor(); // выводим на экран специальное окно и считываем цвет
}

// Слот для кнопки «Отменить»:
void MainWindow::Annul()
{
    DrawBuffer.removeLast();  // удаляем последний элемент из списка
    update();
}

// Слот для кнопки «Очистить»:
void MainWindow::Clear()
{
    DrawBuffer.clear(); // Очищаем список
    update();
}

// Слот для кнопки «Открыть»
void MainWindow::open()
{
    QFile file("picture.dat");
    file.open(QIODevice::ReadOnly); // Создаем файл для чтения
    QDataStream in(&file); // Создаем поток на основе файла
    in>>DrawBuffer; // считываем список окружностей из потока
    update(); // отображаем окружности в окне
}

// Слот для кнопки «Сохранить»
void MainWindow::save()
{
    QFile file("picture.dat"); // создаем файл для записи
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file); // создаем поток на основе файла:
    out << DrawBuffer; // записываем окружности в файл
    file.close(); // обязательно закрываем файл
}
