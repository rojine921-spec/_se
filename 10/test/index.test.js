const test = require('node:test');
const assert = require('node:assert/strict');
const { totals, categorySummary, filterByMonth, toCSV } = require('../src');
const records = [
  { date: '2026-11-01', type: 'income', category: '薪資', amount: 30000 },
  { date: '2026-11-02', type: 'expense', category: '餐飲', amount: 200, note: '午餐' },
  { date: '2026-10-31', type: 'expense', category: '交通', amount: 100 }
];
test('calculates totals', () => assert.deepEqual(totals(records), {income: 30000, expense: 300, balance: 29700}));
test('groups expenses', () => assert.deepEqual(categorySummary(records), {餐飲: 200, 交通: 100}));
test('filters by month', () => assert.equal(filterByMonth(records, '2026-11').length, 2));
test('exports csv', () => assert.match(toCSV(records), /午餐/));
test('rejects invalid entries', () => assert.throws(() => totals([{type:'expense',amount:-1}])));
