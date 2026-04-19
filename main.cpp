#include "mainwindow.h"

#include <QApplication>   // Главный класс приложения.
#include <QLocale>        // Информация о локали системы.
#include <QTranslator>    // Загрузчик переводов.

int main(int argc, char *argv[])
{
    QApplication a(argc, argv); // Создаём объект приложения. Аргументы командной строки передаются ему.

    // --- Поддержка перевода интерфейса ---
    QTranslator translator; // Объект для загрузки файлов перевода.
    // Получаем список предпочитаемых языков системы (например, "ru-RU", "en-US").
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        // Формируем имя файла перевода: "CollectivePixelArt_ru_RU".
        const QString baseName = "CollectivePixelArt_" + QLocale(locale).name();
        // Пытаемся загрузить перевод из ресурсов (путь :/i18n/ задаётся в .qrc или системе сборки).
        if (translator.load(":/i18n/" + baseName)) {
            // Если загрузился, устанавливаем транслятор в приложение.
            a.installTranslator(&translator);
            break;
        }
    }

    // Создаём главное окно.
    MainWindow w;
    w.show(); // Показываем его.

    // Запускаем цикл обработки событий. Функция exec() вернёт управление, когда приложение завершится.
    return QCoreApplication::exec();
}