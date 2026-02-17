-- FinanceServer bootstrap schema (minimal tables required by current report queries)

CREATE TABLE IF NOT EXISTS companies (
  id BIGINT PRIMARY KEY,
  name VARCHAR(255) NOT NULL
);

CREATE TABLE IF NOT EXISTS projects (
  id BIGINT PRIMARY KEY,
  company_id BIGINT NULL,
  name VARCHAR(255) NOT NULL
);

CREATE TABLE IF NOT EXISTS cost_benefit_monthly (
  id BIGINT AUTO_INCREMENT PRIMARY KEY,
  company_id BIGINT NOT NULL,
  project_id BIGINT NOT NULL,
  month DATE NOT NULL,
  output_value DECIMAL(18,2) DEFAULT 0,
  tax DECIMAL(18,2) DEFAULT 0,
  material_cost DECIMAL(18,2) DEFAULT 0,
  machine_cost DECIMAL(18,2) DEFAULT 0,
  machine_depr_cost DECIMAL(18,2) DEFAULT 0,
  labor_mgmt_cost DECIMAL(18,2) DEFAULT 0,
  labor_project_cost DECIMAL(18,2) DEFAULT 0,
  other_cost DECIMAL(18,2) DEFAULT 0,
  finance_fee DECIMAL(18,2) DEFAULT 0,
  nonprod_income DECIMAL(18,2) DEFAULT 0,
  nonprod_expense DECIMAL(18,2) DEFAULT 0,
  income_tax DECIMAL(18,2) DEFAULT 0,
  assess_profit DECIMAL(18,2) DEFAULT 0,
  remark VARCHAR(500) DEFAULT ''
);

CREATE TABLE IF NOT EXISTS ap_accrual (
  id BIGINT AUTO_INCREMENT PRIMARY KEY,
  company_id BIGINT NOT NULL,
  project_id BIGINT NOT NULL,
  vendor_name VARCHAR(255) NOT NULL,
  biz_type VARCHAR(100) NOT NULL,
  amount DECIMAL(18,2) DEFAULT 0,
  biz_date DATE NOT NULL
);

CREATE TABLE IF NOT EXISTS ap_payment (
  id BIGINT AUTO_INCREMENT PRIMARY KEY,
  company_id BIGINT NOT NULL,
  project_id BIGINT NOT NULL,
  vendor_name VARCHAR(255) NOT NULL,
  biz_type VARCHAR(100) NOT NULL,
  amount DECIMAL(18,2) DEFAULT 0,
  pay_date DATE NOT NULL
);

-- Optional seed data
INSERT INTO companies(id, name) VALUES (1, '演示公司A')
  ON DUPLICATE KEY UPDATE name=VALUES(name);

INSERT INTO projects(id, company_id, name) VALUES (1, 1, '演示项目A')
  ON DUPLICATE KEY UPDATE name=VALUES(name), company_id=VALUES(company_id);
