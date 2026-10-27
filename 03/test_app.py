import unittest, tempfile
from pathlib import Path
import app
class ExpenseTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.old=app.DB;app.DB=Path(self.temp.name)/'test.db'
    def tearDown(self):app.DB=self.old;self.temp.cleanup()
    def test_category(self):
        self.assertEqual(app.predict_category('午餐便當'),'餐飲')
        self.assertEqual(app.predict_category('搭捷運'),'交通')
        self.assertEqual(app.predict_category('隨便記錄'),'其他')
    def test_summary(self):
        with app.connect() as c:
            c.execute('INSERT INTO expenses(spend_date,amount,note,category) VALUES (?,?,?,?)',('2026-10-27',120,'便當','餐飲'))
            c.execute('INSERT INTO budgets(month,amount) VALUES (?,?)',('2026-10',100))
        x=app.summary('2026-10');self.assertEqual(x['total'],120);self.assertEqual(x['count'],1);self.assertEqual(x['remaining'],-20)
        self.assertTrue(any('預算' in i for i in x['insights']))
    def test_dates(self):
        self.assertFalse(app.validate_date('2026-02-30'));self.assertTrue(app.validate_date('2026-10-27'))
    def test_month(self):
        self.assertFalse(app.month_ok('2026-13'));self.assertTrue(app.month_ok('2026-10'))
if __name__=='__main__':unittest.main()
