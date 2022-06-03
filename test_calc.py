import unittest
import lesson

class Test(unittest.TestCase):
    def test_sum(self):
        self.assertEqual(lesson.add(4, 7), 11)

    def test_sub(self):
        self.assertEqual(lesson.subtract(4, 7), -3)

    def test_mult(self):
        self.assertEqual(lesson.multiply(4, 7), 28)

    def test_div(self):
        self.assertEqual(lesson.divide(15, 5), 3)

    def test_divide_raises_error(self):
        with self.assertRaises(ZeroDivisionError):
            lesson.divide(10, 0)
