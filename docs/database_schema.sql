-- Enable foreign key support
PRAGMA foreign_keys = ON;

-- Сначала создаем таблицы, на которые будут ссылаться другие:
-- Brokers table
CREATE TABLE IF NOT EXISTS Brokers (
    surname TEXT PRIMARY KEY NOT NULL,
    address TEXT,
    birth_year INTEGER
);

-- Suppliers table (assuming name is unique identifier)
CREATE TABLE IF NOT EXISTS Suppliers (
    supplier_name TEXT PRIMARY KEY NOT NULL,
    contact_info TEXT -- Example additional field
);

-- Buyers table (assuming name is unique identifier)
CREATE TABLE IF NOT EXISTS Buyers (
    buyer_name TEXT PRIMARY KEY NOT NULL,
    address TEXT -- Example additional field
);

-- Теперь таблицы, которые ссылаются на предыдущие:
-- User table for application access
CREATE TABLE IF NOT EXISTS Users (
    user_id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT UNIQUE NOT NULL,
    password_hash TEXT NOT NULL, -- Store hashes, not plain text!
    role TEXT NOT NULL CHECK(role IN ('admin', 'broker')), -- 'admin' for management, 'broker' for brokers
    broker_surname_fk TEXT, -- Link to broker if role is 'broker'
    FOREIGN KEY (broker_surname_fk) REFERENCES Brokers(surname) ON DELETE SET NULL ON UPDATE CASCADE
);

-- Goods table
CREATE TABLE IF NOT EXISTS Goods (
    good_id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    type_of_good TEXT,
    price REAL NOT NULL CHECK(price > 0),
    supplier_name_fk TEXT NOT NULL,
    expiry_date TEXT, -- Format YYYY-MM-DD
    quantity INTEGER NOT NULL CHECK(quantity >= 0),
    FOREIGN KEY (supplier_name_fk) REFERENCES Suppliers(supplier_name) ON DELETE RESTRICT ON UPDATE CASCADE,
    -- Prevent deleting a supplier if they have goods listed
    UNIQUE (name, supplier_name_fk) -- A supplier provides a unique good name
);

-- Deals table
CREATE TABLE IF NOT EXISTS Deals (
    deal_id INTEGER PRIMARY KEY AUTOINCREMENT,
    deal_date TEXT NOT NULL, -- Format YYYY-MM-DD
    good_name_fk TEXT NOT NULL,
    supplier_name_fk TEXT NOT NULL, -- Need this to uniquely identify the good sold
    type_of_good TEXT, -- Can be derived from Goods, but denormalized as per description
    sell_quantity INTEGER NOT NULL CHECK(sell_quantity > 0),
    broker_surname_fk TEXT NOT NULL,
    buyer_name_fk TEXT NOT NULL,
    -- Теперь все внешние ключи ссылаются на уже созданные таблицы
    FOREIGN KEY (broker_surname_fk) REFERENCES Brokers(surname) ON DELETE RESTRICT ON UPDATE CASCADE,
    FOREIGN KEY (buyer_name_fk) REFERENCES Buyers(buyer_name) ON DELETE RESTRICT ON UPDATE CASCADE,
    FOREIGN KEY (good_name_fk, supplier_name_fk) REFERENCES Goods(name, supplier_name_fk) ON DELETE RESTRICT ON UPDATE CASCADE
    -- Prevent deleting brokers, buyers, or goods involved in deals
);

-- Broker statistics table (for Task 4)
CREATE TABLE IF NOT EXISTS BrokerStats (
    stat_id INTEGER PRIMARY KEY AUTOINCREMENT,
    broker_surname_fk TEXT NOT NULL UNIQUE,
    total_sold_units INTEGER DEFAULT 0,
    total_deal_sum REAL DEFAULT 0.0,
    last_updated TEXT, -- Track when it was last calculated
    FOREIGN KEY (broker_surname_fk) REFERENCES Brokers(surname) ON DELETE CASCADE ON UPDATE CASCADE
    -- If broker is deleted, delete their stats
);

-- Optional: Indexes for performance (можно создавать в конце)
CREATE INDEX IF NOT EXISTS idx_deals_date ON Deals(deal_date);
CREATE INDEX IF NOT EXISTS idx_goods_supplier ON Goods(supplier_name_fk);
CREATE INDEX IF NOT EXISTS idx_deals_broker ON Deals(broker_surname_fk);
CREATE INDEX IF NOT EXISTS idx_deals_good_supplier ON Deals(good_name_fk, supplier_name_fk);

-- Add initial admin user (example - use a proper hash!)
-- Эта команда теперь будет работать, так как Users создается после Brokers
INSERT OR IGNORE INTO Users (username, password_hash, role) VALUES ('admin', 'hashed_password123', 'admin');
-- Add example broker user (example - use a proper hash!)
-- INSERT OR IGNORE INTO Brokers (surname, address, birth_year) VALUES ('Ivanov', 'Main St 1', 1980); -- Должно быть перед INSERT Users, если нужно сразу связать
-- INSERT OR IGNORE INTO Users (username, password_hash, role, broker_surname_fk) VALUES ('broker_ivanov', 'broker_password_hash', 'broker', 'Ivanov');