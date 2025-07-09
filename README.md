# click-monitoring-system
Система мониторинга нажатий кнопки, нужная для отслеживания количества нажатий по дням, может быть интегрирована в различные области, включая АСУ ТП
Схема, отображающая состав и взаимодействие компанентов системы:
![image](https://github.com/user-attachments/assets/ac28a663-091f-4116-88a3-88366500f33a)

Диаграмма вариантов использования:
![image](https://github.com/user-attachments/assets/98beaa01-28aa-404a-9a5b-398ca76ae29c)

В ветке приложено три файла: скетч для загрузки в ардуино, серверное приложение, пользовательское приложение на QT. 
СУБД - PostgreSQL, рекомендуемая команда для создания БД:
CREATE TABLE button_clicks (
    id SERIAL PRIMARY KEY,
    button_code INT NOT NULL,
    click_date TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);
