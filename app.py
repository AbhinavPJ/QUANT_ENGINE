from flask import Flask, render_template, request
import pandas as pd
import os
import matplotlib.pyplot as plt
from io import BytesIO
import base64
app = Flask(__name__)
history = dict()
folder_path = "individual_stocks_5yr/individual_stocks_5yr/"
for root, dirs, files in os.walk(folder_path):
    for filename in files:
        if filename.endswith('.csv'):
            try:
                history[filename[:-9]] = pd.read_csv(os.path.join(root, filename), parse_dates=['date'])
            except Exception as e:
                print(f"Error reading {filename}: {e}")
@app.route('/', methods=['GET', 'POST'])
def index():
    chart = None
    error = None
    if request.method == 'POST':
        stock_code = request.form.get('stock_code')
        if stock_code in history:
            df = history[stock_code]
            plt.figure(figsize=(12, 6))
            plt.plot(df['date'], df['close'], linewidth=1, color='blue')
            plt.title(f"{stock_code.upper()} Price Over Time")
            plt.xlabel("Date")
            plt.ylabel("Price")
            plt.grid(True)
            plt.xticks(rotation=45)
            plt.tight_layout()

            # Convert plot to PNG image for web
            buf = BytesIO()
            plt.savefig(buf, format="png")
            buf.seek(0)
            chart = base64.b64encode(buf.getvalue()).decode('utf-8')
            plt.close()
        else:
            error = "NO SUCH STOCK"
    return render_template('index.html', chart=chart, error=error)
if __name__ == '__main__':
    app.run(debug=True)
