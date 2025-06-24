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
path="financials/financials.csv"
df=pd.read_csv(path)
print(df.head())
folder= ""
@app.route('/', methods=['GET', 'POST'])
def index():
    chart = None
    error = None
    stock_info = []
    if request.method == 'POST':
        stock_codes = request.form.get('stock_code').upper().split(',')
        stock_codes = [s.strip() for s in stock_codes if s.strip() != '']
        plot_type = request.form.get('plot_type', 'price')
        valid_codes = [code for code in stock_codes if code in history]
        invalid_codes = [code for code in stock_codes if code not in history]
        if valid_codes:
            plt.figure(figsize=(12, 6))
            for code in valid_codes:
                df_plot = history[code].copy()

                if plot_type == "return":
                    # Compute returns using last valid price
                    prices = df_plot['close']
                    returns = []
                    last_price = None
                    for price in prices:
                        if pd.notna(price):
                            if last_price is None:
                                returns.append(None)
                            else:
                                returns.append(price / last_price)
                            last_price = price
                        else:
                            returns.append(None)
                    df_plot['returns'] = returns
                    plt.plot(df_plot['date'], df_plot['returns'], label=f"{code} (Return)")
                else:
                    plt.plot(df_plot['date'], df_plot['close'], label=f"{code} (Price)")
                try:
                    row = df[df['Symbol'] == code].iloc[0]
                    stock_info.append({
                        'Symbol': code,
                        'Name': row['Name'],
                        'Sector': row['Sector'],
                        'Price': row['Price'],
                        'P/E': row['Price/Earnings'],
                        'Dividend Yield': row['Dividend Yield'],
                        'EPS': row['Earnings/Share'],
                        '52W Low': row['52 Week Low'],
                        '52W High': row['52 Week High'],
                        'Market Cap': row['Market Cap'],
                        'EBITDA': row['EBITDA'],
                        'Price/Sales': row['Price/Sales'],
                        'Price/Book': row['Price/Book'],
                        'SEC Filings': row['SEC Filings']
                    })
                except Exception as e:
                    print(f"Missing financial data for {code}: {e}")

            plt.title("Stock " + ("Returns" if plot_type == "return" else "Prices") + " Over Time")
            plt.xlabel("Date")
            plt.ylabel("Return" if plot_type == "return" else "Price")
            plt.grid(True)
            plt.xticks(rotation=45)
            plt.tight_layout()
            plt.legend()
            buf = BytesIO()
            plt.savefig(buf, format="png")
            buf.seek(0)
            chart = base64.b64encode(buf.getvalue()).decode('utf-8')
            plt.close()
        else:
            error = "No valid stock codes entered."
        if invalid_codes:
            if error:
                error += " | "
            else:
                error = ""
            error += f"Invalid: {', '.join(invalid_codes)}"
    return render_template('index.html', chart=chart, error=error, stock_info=stock_info)
@app.route('/screener', methods=['GET', 'POST'])
def screener():
    results = None
    error = None
    columns = df.select_dtypes(include=['number']).columns.tolist()  # only numeric filters allowed
    sort_col = None
    order = 'desc'

    if request.method == 'POST':
        try:
            filters = []
            for i in range(len(request.form.getlist('column'))):
                col = request.form.getlist('column')[i]
                op = request.form.getlist('operator')[i]
                val = request.form.getlist('value')[i]
                if col and op and val:
                    filters.append((col, op, val))
            filtered = df.copy()
            for col, op, val in filters:
                try:
                    val = float(val)
                    if op == '<':
                        filtered = filtered[filtered[col] < val]
                    elif op == '<=':
                        filtered = filtered[filtered[col] <= val]
                    elif op == '>':
                        filtered = filtered[filtered[col] > val]
                    elif op == '>=':
                        filtered = filtered[filtered[col] >= val]
                    elif op == '==':
                        filtered = filtered[filtered[col] == val]
                except:
                    continue 
            sort_col = request.form.get('sort_by')
            order = request.form.get('order')
            if sort_col in df.columns:
                filtered = filtered.sort_values(by=sort_col, ascending=(order == 'asc'))
            results = filtered.to_dict(orient='records')
        except Exception as e:
            error = f"Error applying filters: {e}"
    return render_template('screener.html', results=results, error=error, columns=columns)
if __name__ == '__main__':
    app.run(debug=True)
