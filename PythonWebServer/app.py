from flask import Flask, request, render_template
import csv

app = Flask(__name__)


@app.route('/')
def index():
    return render_template("index.html")


@app.route('/postjson', methods=['POST'])
def post_json_handler():
    content = request.get_json()
    accelerometer = content['accelerometer']
    magnetometer = content['magnetometer']
    gyroscope = content['gyroscope']
    print(accelerometer)
    print(magnetometer)
    print(gyroscope)
    # generate csv
    save_to_csv(accelerometer, magnetometer, gyroscope)
    return 'JSON POSTED'


def save_to_csv(accelerometer, magnetometer, gyroscope):
    outfile = open("ESP32_LSM9DS1IMU.csv", "w")
    writer = csv.writer(outfile)
    writer.writerow(accelerometer)
    writer.writerow(magnetometer)
    writer.writerow(gyroscope)


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8080)
