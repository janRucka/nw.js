import time
import os

from selenium import webdriver
from selenium.webdriver.chrome.options import Options
chrome_options = Options()
chrome_options.add_argument("nwapp=" + os.path.dirname(os.path.abspath(__file__)) + "\..\javascript")

driver = webdriver.Chrome(executable_path=os.environ['CHROMEDRIVER'], chrome_options=chrome_options)
#driver.implicitly_wait(5)
time.sleep(1)
try:
    driver.implicitly_wait(10) # 10s timeout when finding an element

    clickme = driver.find_element_by_id('downloadIdButton')
    clickme.click() # click the button

    result = driver.find_element_by_id('downloadResult')
    assert("success" in result.get_attribute('innerHTML'))
finally:
    driver.quit()