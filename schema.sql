-- 1. Таблица маклеров
CREATE TABLE Brokers
(
    surname VARCHAR(50) PRIMARY KEY,
    address VARCHAR(100) NOT NULL,
    year_of_birth INT NOT NULL
);

-- 2. Таблица фирм-поставщиков
CREATE TABLE Suppliers
(
    name VARCHAR(50) PRIMARY KEY,
    address VARCHAR(100) NOT NULL
);

-- 3. Таблица фирм-покупателей
CREATE TABLE Buyers
(
    name VARCHAR(50) PRIMARY KEY,
    address VARCHAR(100) NOT NULL
);

-- 4. Таблица товаров
CREATE TABLE Goods
(
    name VARCHAR(50),
    type VARCHAR(50),
    price INT NOT NULL,
    supplier_name VARCHAR(50) NOT NULL,
    expiration_date DATE NOT NULL,
    quantity INT NOT NULL,
    PRIMARY KEY (name, supplier_name),
    FOREIGN KEY (supplier_name) REFERENCES Suppliers(name)
);

-- 5. Таблица сделок
CREATE TABLE Deals
(
    deal_date DATE NOT NULL,
    good_name VARCHAR(50),
    type_of_good VARCHAR(50),
    sell_quantity INT NOT NULL,
    broker_surname VARCHAR(50),
    buyer_name VARCHAR(50),
    supplier_name VARCHAR(50),
    PRIMARY KEY (deal_date, good_name, broker_surname, buyer_name, supplier_name),
    FOREIGN KEY (broker_surname) REFERENCES Brokers(surname),
    FOREIGN KEY (buyer_name) REFERENCES Buyers(name),
    FOREIGN KEY (good_name, supplier_name) REFERENCES Goods(name, supplier_name)
);

-- 6. Таблица статистики маклеров (*)
CREATE TABLE BrokerStats
(
    broker_surname VARCHAR(50) PRIMARY KEY,
    total_sold_units INT DEFAULT 0,
    total_deal_sum INT DEFAULT 0,
    FOREIGN KEY (broker_surname) REFERENCES Brokers(surname)
);