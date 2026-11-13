'use strict';

function validateTransactions(transactions) {
  if (!Array.isArray(transactions)) throw new TypeError('transactions must be an array');
  for (const t of transactions) {
    if (!t || !['income', 'expense'].includes(t.type)) {
      throw new TypeError('Each transaction type must be income or expense');
    }
    if (typeof t.amount !== 'number' || !Number.isFinite(t.amount) || t.amount < 0) {
      throw new TypeError('Transaction amount must be a finite non-negative number');
    }
  }
}

function totals(transactions) {
  validateTransactions(transactions);
  const result = { income: 0, expense: 0, balance: 0 };
  for (const t of transactions) result[t.type] += t.amount;
  result.balance = result.income - result.expense;
  return result;
}

function categorySummary(transactions, type = 'expense') {
  validateTransactions(transactions);
  if (!['income', 'expense'].includes(type)) throw new TypeError('type must be income or expense');
  const result = {};
  for (const t of transactions) {
    if (t.type !== type) continue;
    const category = String(t.category ?? '未分類');
    result[category] = (result[category] || 0) + t.amount;
  }
  return result;
}

function filterByMonth(transactions, month) {
  validateTransactions(transactions);
  if (typeof month !== 'string' || !/^\d{4}-(0[1-9]|1[0-2])$/.test(month)) {
    throw new TypeError('month must use YYYY-MM format');
  }
  return transactions.filter(t => typeof t.date === 'string' && t.date.slice(0, 7) === month);
}

function csvCell(value) {
  let s = String(value ?? '');
  if (/^[\s]*[=+@\-\t\r]/.test(s)) s = "'" + s;
  return '"' + s.replace(/"/g, '""') + '"';
}

function toCSV(transactions) {
  validateTransactions(transactions);
  const rows = [['date', 'type', 'category', 'amount', 'note']];
  for (const t of transactions) {
    rows.push([t.date ?? '', t.type, t.category ?? '', t.amount, t.note ?? '']);
  }
  return '\uFEFF' + rows.map(row => row.map(csvCell).join(',')).join('\r\n');
}

module.exports = { totals, categorySummary, filterByMonth, toCSV };
