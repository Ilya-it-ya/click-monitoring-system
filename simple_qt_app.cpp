#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QFont>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QMap>
#include <QList>
#include <algorithm>

QT_CHARTS_USE_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent)
    {
        setWindowTitle("Визуализация нажатий кнопки: Количество по дням");
        setMinimumSize(1024, 768);

        QFont titleFont = font();
        titleFont.setPointSize(titleFont.pointSize() + 4);
        setFont(titleFont);

        QWidget *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        QVBoxLayout *layout = new QVBoxLayout(centralWidget);

        QPushButton *refreshButton = new QPushButton("Обновить график из БД", this);
        refreshButton->setMinimumHeight(50);

        QFont buttonFont = refreshButton->font();
        buttonFont.setPointSize(buttonFont.pointSize() + 2);
        refreshButton->setFont(buttonFont);

        layout->addWidget(refreshButton);

        connect(refreshButton, &QPushButton::clicked, this, &MainWindow::updateChart);

        chart = new QChart();
        chart->setTitle("Количество нажатий кнопки по дням");

        QFont chartTitleFont = chart->titleFont();
        chartTitleFont.setPointSize(chartTitleFont.pointSize() + 2);
        chart->setTitleFont(chartTitleFont);

        chartView = new QChartView(chart, this);
        chartView->setRenderHint(QPainter::Antialiasing);
        layout->addWidget(chartView);

        categoryAxis = new QBarCategoryAxis();
        categoryAxis->setTitleText("Дата");
        valueAxis = new QValueAxis();
        valueAxis->setTitleText("Количество нажатий");
        valueAxis->setLabelFormat("%i");

        chart->addAxis(categoryAxis, Qt::AlignBottom);
        chart->addAxis(valueAxis, Qt::AlignLeft);

        setupDatabaseConnection();

        updateChart();

        qDebug() << "Окно, кнопка и место для графика созданы.";
    }

private slots:
    void updateChart()
    {
        qDebug() << "Кнопка 'Обновить график из БД' нажата. Получаем агрегированные данные из SQL.";

        chart->removeAllSeries();

        if (!db.isOpen() && !db.open()) {
            qCritical() << "Ошибка: Соединение с базой данных не открыто и не удалось открыть. " << db.lastError().text();
            return;
        }

        QSqlQuery query(db);
        QString queryString = "SELECT TO_CHAR(press_timestamp, 'YYYY-MM-DD') AS click_date, COUNT(*) AS click_count "
                              "FROM название бд "
                              /* "WHERE press_timestamp >= NOW() - INTERVAL '7 day' " */
                              "GROUP BY click_date "
                              "ORDER BY click_date ASC";
        //в команде sql нужно вписать название бд
        if (query.exec(queryString)) {
            QBarSet *barSet = new QBarSet("Нажатия за день");
            QStringList categories;
            QMap<QString, qreal> dailyCounts;

            while (query.next()) {
                QString date = query.value("click_date").toString();
                qreal count = query.value("click_count").toReal();
                dailyCounts.insert(date, count);
            }
            
            QList<QString> sortedDates = dailyCounts.keys();
            std::sort(sortedDates.begin(), sortedDates.end());

            for (const QString &date : sortedDates) {
                categories << date;
                barSet->append(dailyCounts.value(date));
            }

            if (dailyCounts.count() > 0) {
                QBarSeries *series = new QBarSeries();
                series->append(barSet);

                chart->addSeries(series);

                categoryAxis->clear();
                categoryAxis->append(categories);
                
                qreal maxCount = 0;
                for (qreal count : dailyCounts.values()) {
                    if (count > maxCount) {
                        maxCount = count;
                    }
                }
                
                if (maxCount > 0) {
                    valueAxis->setRange(0, maxCount * 1.1);
                } else {
                    valueAxis->setRange(0, 10);
                }
                valueAxis->applyNiceNumbers();

                series->attachAxis(categoryAxis);
                series->attachAxis(valueAxis);    

                chart->legend()->setVisible(true);
                chart->legend()->setAlignment(Qt::AlignBottom);

                qDebug() << "Данные успешно получены из БД и график обновлен. Дней с данными:" << dailyCounts.count();
            } else {
                
                qDebug() << "Из БД получено 0 записей. График пуст."; 
                chart->setTitle("Нет данных для отображения.");
                categoryAxis->clear();
                valueAxis->setRange(0, 10);
            }
        } else {
            qCritical() << "Ошибка выполнения запроса:" << query.lastError().text();
        }
    }

private:
    QSqlDatabase db;
    QChart *chart;
    QChartView *chartView;
    QBarCategoryAxis *categoryAxis;
    QValueAxis *valueAxis;

    void setupDatabaseConnection()
    {
        db = QSqlDatabase::addDatabase("QPSQL");
        db.setHostName("localhost");
        db.setPort(5432);
        db.setDatabaseName("название бд"); // сюда вписать название бд
        db.setUserName("имя пользователя"); //сюда вписать имя пользователя
        db.setPassword("пароль"); // сюда вписать пароль пользователя

        if (!db.open()) {
            qCritical() << "Ошибка начального подключения к базе данных:" << db.lastError().text();
        } else {
            qDebug() << "Успешно подключено к базе данных PostgreSQL.";
        }
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}

#include "simple_qt_app.moc"