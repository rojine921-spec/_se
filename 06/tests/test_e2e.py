import threading
from werkzeug.serving import make_server


def test_create_complete_delete_in_headless_browser(app):
    from playwright.sync_api import sync_playwright

    server = make_server('127.0.0.1', 0, app)
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()
    url = f'http://127.0.0.1:{server.server_port}'
    try:
        with sync_playwright() as p:
            browser = p.chromium.launch(headless=True)
            try:
                page = browser.new_page()
                page.goto(url)
                page.get_by_role('textbox', name='待辦事項').fill('完成第七週作業')
                page.get_by_role('button', name='新增').click()
                task = page.get_by_text('完成第七週作業')
                task.wait_for()
                page.get_by_role('checkbox', name='完成 完成第七週作業').check()
                page.locator('span.done').get_by_text('完成第七週作業').wait_for()
                page.get_by_role('button', name='刪除 完成第七週作業').click()
                page.get_by_text('目前沒有待辦事項').wait_for()
            finally:
                browser.close()
    finally:
        server.shutdown()
        thread.join(timeout=5)
