from app import validate_task


def test_valid_title():
    assert validate_task('  買牛奶  ') == (True, '買牛奶')


def test_empty_title():
    assert validate_task('   ')[0] is False


def test_overlength_title():
    assert validate_task('a' * 101)[0] is False


def test_invalid_title_type():
    assert validate_task(123)[0] is False
