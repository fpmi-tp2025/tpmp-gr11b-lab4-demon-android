#ifndef QUERIES_H
#define QUERIES_H

#include <stddef.h> // Needed for size_t in safe_scanf declaration

// +++ Add Declarations for helper functions +++
void safe_scanf(const char *prompt, char *buffer, size_t buffer_size);
int safe_scanf_int(const char *prompt);
// +++ End Additions +++

// --- Task 2 Queries ---
void run_sales_summary_by_period();
void run_buyers_by_good();
void run_most_popular_type_info();
void run_top_broker_info();
void run_supplier_brokers_info();

// --- Task 3 CRUD Operations ---
void add_new_broker();
void add_new_good();
void add_new_deal(); // Consider adding checks (e.g., sufficient quantity)
void update_good_price();
void delete_deal_by_id();
// Add more CRUD as needed (Suppliers, Buyers, Users?)

// --- Task 4, 5, 6 Functions ---
void recalculate_broker_stats(); // Renamed from update_broker_stats for clarity
void update_goods_quantity_and_clear_deals();
void show_deals_on_date();
void show_broker_deals(const char *broker_surname); // For broker role

#endif // QUERIES_H