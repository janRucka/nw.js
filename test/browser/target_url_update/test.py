import time
import os

from selenium import webdriver
from selenium.webdriver.chrome.options import Options
chrome_options = Options()
chrome_options.add_argument("nwapp=" + os.path.dirname(os.path.abspath(__file__)) + "\..\javascript")

driver = webdriver.Chrome(executable_path=os.environ['CHROMEDRIVER'], chrome_options=chrome_options)
time.sleep(1)
try:
    driver.implicitly_wait(30)  # 30s timeout when finding an element

    clickme = driver.find_element_by_id('targetUrlUpdateButton')
    clickme.click() 

    result = driver.find_element_by_id('targetUrlUpdateResult')
    assert("success" in result.get_attribute('innerHTML'))
finally:
    driver.quit()