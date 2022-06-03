def test_request_response():
    response = requests.get("https://bank.gov.ua/NBUStatService/v1/statdirectory/exchange?valcode=")
    assert_true(response.ok)
