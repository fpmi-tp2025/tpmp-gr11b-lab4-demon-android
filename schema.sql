-- Таблица маклеров
CREATE TABLE Broker
(
    broker_id SERIAL PRIMARY KEY,
    last_name VARCHAR(50) NOT NULL,
    address TEXT,
    birth_year INT CHECK (birth_year > 1900)
);

-- Таблица товаров
CREATE TABLE Product
(
    product_id SERIAL PRIMARY KEY,
    product_name VARCHAR(100) NOT NULL,
    product_type VARCHAR(50) NOT NULL,
    unit_price NUMERIC(10,2) NOT NULL CHECK (unit_price > 0),
    supplier VARCHAR(100) NOT NULL,
    expiration_date DATE NOT NULL,
    supplied_units INT NOT NULL CHECK (supplied_units >= 0)
);

-- Таблица заключенных сделок
CREATE TABLE Deal (
    deal_id SERIAL PRIMARY KEY,
    deal_date DATE NOT NULL,
    product_id INT NOT NULL,
    sold_units INT NOT NULL CHECK (sold_units > 0),
    broker_id INT NOT NULL,
    buyer_firm VARCHAR(100) NOT NULL,
    FOREIGN KEY (product_id) REFERENCES Product(product_id)
        ON UPDATE CASCADE ON DELETE RESTRICT,
    FOREIGN KEY
(broker_id) REFERENCES Broker
(broker_id)
        ON
UPDATE CASCADE ON
DELETE RESTRICT
);

-- Таблица статистики маклеров
CREATE TABLE BrokerStatistics
(
    broker_id INT PRIMARY KEY,
    total_sold_units INT NOT NULL DEFAULT 0,
    total_deal_amount NUMERIC(12,2) NOT NULL DEFAULT 0,
    FOREIGN KEY (broker_id) REFERENCES Broker(broker_id)
        ON UPDATE CASCADE ON DELETE CASCADE
);