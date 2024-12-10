from playwright.sync_api import sync_playwright, Playwright
from werkzeug.wrappers import Request, Response
from PIL import Image
import io
import logging
import sys
logging.basicConfig(stream=sys.stdout, level=logging.DEBUG)

def run(p: Playwright):
    browser = p.chromium.launch()
   
    # Open new browser context with HTTP Basic Auth
    context = browser.new_context(
        http_credentials={"username": "USERNAME", "password": "PASSWORD"},
        viewport={ 'width': 270, 'height': 700 }
    )

    page = context.new_page()
    
    page.goto("https://WEBSITETOSCRAPE")

    page.get_by_role("link", name="Auswahl").click()
    page.get_by_text("5d").click()

    js_scipt  = """
    document.querySelector('.ui-content').style.background="#FFF";
    document.querySelector('.ui-content').style.padding="5px";
    document.querySelector('#plan').style.margin="0.3em 0";
    document.querySelectorAll('#plan li').forEach(elem => elem.style.background="#FFF");    
    """
    page.add_script_tag(content=js_scipt)

    element = page.locator('.ui-content')
    image = element.screenshot(path="screenshot.png")
    
    context.close()
    browser.close()

    return image


def get_full_image():
    # Get image of website section
    with sync_playwright() as playwright:
        png_data = run(playwright)

    # Convert image data to Pillow image object
    screenshot = Image.open(io.BytesIO(png_data))

    # The 6 colors of inkplate color
    # https://inkplate.readthedocs.io/en/latest/arduino.html#inkplate-drawbitmap
    palette = [
        0, 0, 0, # black
        255, 255, 255, # white
        0, 255, 0, # green
        0, 0, 255, # blue
        255, 0, 0, # red
        255, 255, 0, # yellow
        255, 153, 0, # orange
    ]

    # Create image with restricted palette mode and white background
    screen_size = (600, 448)
    result_img = Image.new('P', screen_size, 1)
    result_img.putpalette(palette)

    # Quantize the screenshot to the restricted palette
    s_quant = screenshot.quantize(palette=result_img, dither=1)

    # Paste the quantized screenshot into the result image
    crop_size = tuple(map(min, screenshot.size, screen_size))
    box = (0,0) + crop_size
    result_img.paste(s_quant.crop(box), box)

    img_byte_arr = io.BytesIO()
    result_img.save(img_byte_arr, format='PNG')
    return img_byte_arr.getvalue()

def get_playwright_image():
    with sync_playwright() as playwright:
        png_data = run(playwright)
    return png_data

def application(environ, start_response):
    response = Response(get_full_image(), mimetype='application/octet-stream')

    # If no image modifications are needed, you could directly serve the playwright image:
    # response = Response(get_playwright_image(), mimetype='application/octet-stream')    

    return response(environ, start_response)

if __name__ == '__main__':
    from werkzeug.serving import run_simple
    run_simple('0.0.0.0', 5050, application)
    
