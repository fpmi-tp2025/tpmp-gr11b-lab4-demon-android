#include "db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void run_sales_summary_by_period() {
  char start[11], end[11];
  printf("Начальная дата (YYYY-MM-DD): ");
  scanf("%s", start);
  printf("Конечная дата (YYYY-MM-DD): ");
  scanf("%s", end);

  char query[1024];
  snprintf(query, sizeof(query),
           "SELECT good_name, SUM(sell_quantity) AS total_sold, "
           "SUM(sell_quantity * Goods.price) AS total_income "
           "FROM Deals JOIN Goods ON Deals.good_name = Goods.name "
           "AND Deals.supplier_name = Goods.supplier_name "
           "WHERE deal_date BETWEEN '%s' AND '%s' GROUP BY good_name;",
           start, end);

  execute_query(query);
}

void run_buyers_by_good() {
  const char *query =
      "SELECT good_name, buyer_name, SUM(sell_quantity) AS total_units, "
      "SUM(sell_quantity * Goods.price) AS total_cost "
      "FROM Deals JOIN Goods ON Deals.good_name = Goods.name "
      "AND Deals.supplier_name = Goods.supplier_name "
      "GROUP BY good_name, buyer_name;";
  execute_query(query);
}

void run_most_popular_type_info() {
  const char *query =
      "WITH TypeSales AS ("
      "  SELECT type_of_good, SUM(sell_quantity) AS total_sold "
      "  FROM Deals GROUP BY type_of_good "
      "), MaxType AS ("
      "  SELECT type_of_good FROM TypeSales "
      "  WHERE total_sold = (SELECT MAX(total_sold) FROM TypeSales)"
      ") "
      "SELECT buyer_name, type_of_good, SUM(sell_quantity) AS total_units, "
      "SUM(sell_quantity * Goods.price) AS total_cost "
      "FROM Deals "
      "JOIN Goods ON Deals.good_name = Goods.name AND Deals.supplier_name = "
      "Goods.supplier_name "
      "WHERE type_of_good = (SELECT type_of_good FROM MaxType) "
      "GROUP BY buyer_name, type_of_good;";
  execute_query(query);
}

void run_top_broker_info() {
  const char *query =
      "WITH BrokerDeals AS ("
      "  SELECT broker_surname, COUNT(*) AS deal_count "
      "  FROM Deals GROUP BY broker_surname"
      "), TopBroker AS ("
      "  SELECT broker_surname FROM BrokerDeals "
      "  WHERE deal_count = (SELECT MAX(deal_count) FROM BrokerDeals)"
      ") "
      "SELECT Brokers.*, Deals.supplier_name "
      "FROM Brokers "
      "JOIN Deals ON Brokers.surname = Deals.broker_surname "
      "WHERE Brokers.surname = (SELECT broker_surname FROM TopBroker) "
      "GROUP BY Brokers.surname, Deals.supplier_name;";
  execute_query(query);
}

void run_supplier_brokers_info() {
  const char *query =
      "SELECT supplier_name, broker_surname, SUM(sell_quantity) AS total_sold, "
      "SUM(sell_quantity * Goods.price) AS total_value "
      "FROM Deals "
      "JOIN Goods ON Deals.good_name = Goods.name AND Deals.supplier_name = "
      "Goods.supplier_name "
      "GROUP BY supplier_name, broker_surname;";
  execute_query(query);
}

void update_broker_stats() {
  const char *delete_query = "DELETE FROM BrokerStats;";
  execute_query(delete_query);

  const char *insert_query =
      "INSERT INTO BrokerStats (broker_surname, total_sold_units, "
      "total_deal_sum) "
      "SELECT broker_surname, SUM(sell_quantity), "
      "SUM(sell_quantity * Goods.price) "
      "FROM Deals JOIN Goods ON Deals.good_name = Goods.name "
      "AND Deals.supplier_name = Goods.supplier_name "
      "GROUP BY broker_surname;";
  execute_query(insert_query);

  printf("Статистика маклеров обновлена.\n");
}

void update_goods_quantity_and_clear_deals() {
  char date[11];
  printf("Введите дату (YYYY-MM-DD): ");
  scanf("%s", date);

  char update_query[1024];
  snprintf(update_query, sizeof(update_query),
           "UPDATE Goods SET quantity = quantity - IFNULL((SELECT "
           "SUM(sell_quantity) "
           "FROM Deals WHERE Deals.good_name = Goods.name AND "
           "Deals.supplier_name = Goods.supplier_name "
           "AND deal_date <= '%s'), 0);",
           date);
  execute_query(update_query);

  char delete_query[256];
  snprintf(delete_query, sizeof(delete_query),
           "DELETE FROM Deals WHERE deal_date <= '%s';", date);
  execute_query(delete_query);

  printf("Обновление остатков и очистка сделок завершены.\n");
}

void show_deals_on_date() {
  char date[11];
  printf("Введите дату (YYYY-MM-DD): ");
  scanf("%s", date);

  char query[512];
  snprintf(query, sizeof(query), "SELECT * FROM Deals WHERE deal_date = '%s';",
           date);
  execute_query(query);
}