# Display ChurchSuite rota contents on a Inkplate e-paper display
This code was forked from xxx and changed to access the ChurchSuite rotas module to display the keyholders for a specific day.
As such there are several secrets, and several secrets. Specifics:
- module access endpoint URL
- specific rota name
The secrets are:
- wifi SSID and password
- ChurchSuite userid and password

  Other than these, the code is reasonably generic.
  ---
  **Note:** as of March 2025, ChurchSuite does not expose the Rotas module as an endpoint in their v2 API, so this code of necessity exports a CSV file and converts this to XML
