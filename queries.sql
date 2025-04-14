-- Параметры: @start_date, @end_date
SELECT
    p.product_name,
    SUM(d.sold_units) AS total_sold_units,
    SUM(d.sold_units * p.unit_price) AS total_sale_value
FROM Deal d
    JOIN Product p ON d.product_id = p.product_id
WHERE d.deal_date BETWEEN @start_date AND @end_date
GROUP BY p.product_name;



SELECT
    p.product_name,
    d.buyer_firm,
    SUM(d.sold_units) AS units_bought,
    SUM(d.sold_units * p.unit_price) AS sale_value
FROM Deal d
    JOIN Product p ON d.product_id = p.product_id
GROUP BY p.product_name, d.buyer_firm;



-- Сначала определяем тип товара с максимальным спросом
WITH
    Demand
    AS
    (
        SELECT
            p.product_type,
            SUM(d.sold_units) AS total_units
        FROM Deal d
            JOIN Product p ON d.product_id = p.product_id
        GROUP BY p.product_type
    ),
    MaxDemand
    AS
    (
        SELECT product_type
        FROM Demand
        WHERE total_units = (SELECT MAX(total_units)
        FROM Demand)
    )
SELECT
    p.product_type,
    d.buyer_firm,
    SUM(d.sold_units) AS units_bought,
    SUM(d.sold_units * p.unit_price) AS sale_value
FROM Deal d
    JOIN Product p ON d.product_id = p.product_id
WHERE p.product_type IN (SELECT product_type
FROM MaxDemand)
GROUP BY p.product_type, d.buyer_firm;



WITH
    BrokerDeals
    AS
    (
        SELECT
            broker_id,
            COUNT(*) AS deal_count
        FROM Deal
        GROUP BY broker_id
    ),
    MaxBroker
    AS
    (
        SELECT broker_id
        FROM BrokerDeals
        WHERE deal_count = (SELECT MAX(deal_count)
        FROM BrokerDeals)
    )
SELECT
    b.last_name,
    b.address,
    b.birth_year,
    -- Выбираем уникальные поставщики для сделок данного маклера:
    ARRAY_AGG(DISTINCT p.supplier) AS supplier_list
FROM Deal d
    JOIN Broker b ON d.broker_id = b.broker_id
    JOIN Product p ON d.product_id = p.product_id
WHERE d.broker_id IN (SELECT broker_id
FROM MaxBroker)
GROUP BY b.broker_id;



SELECT
    p.supplier,
    b.last_name,
    SUM(d.sold_units) AS total_units_sold,
    SUM(d.sold_units * p.unit_price) AS total_sale_value
FROM Deal d
    JOIN Product p ON d.product_id = p.product_id
    JOIN Broker b ON d.broker_id = b.broker_id
GROUP BY p.supplier, b.last_name;