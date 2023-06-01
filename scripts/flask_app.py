from flask import Flask, send_from_directory, request

app = Flask(__name__)

@app.route('/upload', methods=['POST'])
def upload_file():
    file = request.files['file']
    if file:
        dest_path = '/workspaces/qrypt-security-quickstarts-cpp/' + file.filename
        file.save(dest_path)
        return dest_path
    else:
        return 'No file received.'

if __name__ == '__main__':
    app.run(debug=True, port=5000, host='0.0.0.0')
