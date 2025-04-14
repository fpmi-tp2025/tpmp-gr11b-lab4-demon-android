-- Файл: docs/seed_data.sql
-- Заполняем таблицы тестовыми данными

-- Игнорируем ошибки, если записи уже существуют (на случай повторного запуска)
PRAGMA ignore_check_constraints = ON;

-- Поставщики
INSERT OR IGNORE INTO Suppliers (supplier_name, contact_info) VALUES
('Aromashka Inc.', 'contact@aromashka.com'),
('Parfum Lux', 'info@parfumlux.org'),
('Sweet Scents Co.', 'sales@sweetscents.co');

-- Покупатели
INSERT OR IGNORE INTO Buyers (buyer_name, address) VALUES
('Beauty World', '123 Main St, City A'),
('Fragrance Hub', '45 Market Ave, City B'),
('Scent Style', '789 Oak Ln, City C');

-- Брокеры
INSERT OR IGNORE INTO Brokers (surname, address, birth_year) VALUES
('Petrov', 'Green Street 5, City A', 1985),
('Sidorov', 'Blue Avenue 10, City B', 1992),
('Kim', 'Red Square 1, City C', 1988);

-- Пользователи (связываем с брокерами, используем хеши-заглушки)
-- Пароль для broker_petrov: "petrovpass"
-- Пароль для broker_sidorov: "sidorovpass"
INSERT OR IGNORE INTO Users (username, password_hash, role, broker_surname_fk) VALUES
('broker_petrov', 'hashed_petrovpass', 'broker', 'Petrov'),
('broker_sidorov', 'hashed_sidorovpass', 'broker', 'Sidorov');
-- Администратор уже должен быть добавлен из database_schema.sql

-- Товары
INSERT OR IGNORE INTO Goods (name, type_of_good, price, supplier_name_fk, expiry_date, quantity) VALUES
('Eau de Lune', 'Eau de Parfum', 75.50, 'Aromashka Inc.', '2026-12-31', 100),
('Ocean Breeze', 'Eau de Toilette', 45.00, 'Parfum Lux', '2025-06-30', 150),
('Floral Dream', 'Eau de Parfum', 82.00, 'Aromashka Inc.', NULL, 80), -- Без срока годности
('Spicy Night', 'Eau de Cologne', 55.90, 'Sweet Scents Co.', '2026-01-15', 120),
('Citrus Burst', 'Eau de Toilette', 49.99, 'Parfum Lux', '2025-10-01', 200);

-- Сделки (убедитесь, что даты, товары, брокеры, покупатели существуют)
-- Используем разные даты для тестов
INSERT OR IGNORE INTO Deals (deal_date, good_name_fk, supplier_name_fk, type_of_good, sell_quantity, broker_surname_fk, buyer_name_fk) VALUES
('2025-04-10', 'Eau de Lune', 'Aromashka Inc.', 'Eau de Parfum', 5, 'Petrov', 'Beauty World'),
('2025-04-11', 'Ocean Breeze', 'Parfum Lux', 'Eau de Toilette', 10, 'Sidorov', 'Fragrance Hub'),
('2025-04-12', 'Spicy Night', 'Sweet Scents Co.', 'Eau de Cologne', 8, 'Petrov', 'Scent Style'),
('2025-04-12', 'Eau de Lune', 'Aromashka Inc.', 'Eau de Parfum', 3, 'Kim', 'Fragrance Hub'), -- Добавим брокера Kim (если вы его создали в Brokers)
('2025-04-13', 'Citrus Burst', 'Parfum Lux', 'Eau de Toilette', 15, 'Sidorov', 'Beauty World'),
('2025-04-14', 'Ocean Breeze', 'Parfum Lux', 'Eau de Toilette', 7, 'Petrov', 'Scent Style');

PRAGMA ignore_check_constraints = OFF; -- Включаем проверки обратно (хотя для INSERT OR IGNORE это не так важно)