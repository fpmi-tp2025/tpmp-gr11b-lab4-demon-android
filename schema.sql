CREATE TABLE Brokers
(
    surname CHAR(50) PRIMARY KEY,
    adress CHAR(100) NOT NULL,
    year_of_birth INT NOT NULL
);

CREATE TABLE Goods
(
    name CHAR(50),
    type CHAR(50),
    price INT NOT NULL,
    vendor CHAR(50),
    expiration_date DATE NOT NULL,
    num INT NOT NULL,
    PRIMARY KEY (name, vendor),
    UNIQUE(name, vendor)
);

CREATE TABLE Deals
(
    deal_date DATE NOT NULL,
    good_name CHAR(50),
    type_of_good CHAR(50),
    sell_num INT NOT NULL,
    broker_surname CHAR(50),
    vendor CHAR(50),
    PRIMARY KEY (deal_date, good_name, broker_surname, vendor),
    FOREIGN KEY (broker_surname) REFERENCES Brokers(surname),
    FOREIGN KEY (good_name, vendor) REFERENCES Goods(name, vendor)
);