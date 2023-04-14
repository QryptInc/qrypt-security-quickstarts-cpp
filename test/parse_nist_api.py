import requests
import json

if __name__ == '__main__' :
    resp = requests.get('https://nist.qrypt.com/api/v1/logs')
    assert resp.status_code < 400, f"Could not get NIST logs; got error code {resp.status_code}!"
    results = {}
    try:
        results = resp.json()
    except json.decoder.JSONDecodeError as e: 
        print(f"No test results in NIST response!")
        raise
    num_success = 0
    latest_success = ""
    latest_fail = ""
    for result in results:
        time_string = result['time_of_completion_string']
        if result['nist_all_passed']:
            num_success += 1
            if not latest_success:
                latest_success = time_string
        elif not latest_fail:
            latest_fail = time_string
    assert num_success > 2, f"Too many NIST failures! Latest failure reported at {latest_fail}!"
    print("Qrypt EaaS API passes NIST Statistical Test Suite - entropy is sufficiently random for cryptographic applications!")
    print(f"Number of NIST suite runs reported in last hour: {len(results)}")
    print(f"Most recent successful test completed at: {latest_success}")
