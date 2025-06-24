import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Load PnL
df = pd.read_csv("pnl.csv")

# Compute daily returns
df["daily_return"] = df["wealth"].pct_change()

# Compute metrics
mean_return = df["daily_return"].mean()
std_return = df["daily_return"].std()
sharpe_ratio = (mean_return / std_return) * np.sqrt(252) if std_return > 0 else 0

# Max drawdown
rolling_max = df["wealth"].cummax()
drawdown = (df["wealth"] - rolling_max) / rolling_max
max_drawdown = drawdown.min()

# Total return
total_return = df["wealth"].iloc[-1] / df["wealth"].iloc[0] - 1

# Display metrics
print("📊 Strategy Metrics:")
print(f"Total Return:     {total_return * 100:.2f}%")
print(f"Sharpe Ratio:     {sharpe_ratio:.4f}")
print(f"Max Drawdown:     {max_drawdown * 100:.2f}%")
print(f"Mean Daily Return:{mean_return * 100:.4f}%")
print(f"Volatility:       {std_return * 100:.4f}%")

# Plot wealth over time
plt.figure(figsize=(12, 6))
plt.plot(df["day"], df["wealth"], color="skyblue", linewidth=2)
plt.title("Wealth Over Time")
plt.xlabel("Day")
plt.ylabel("Wealth")
plt.grid(True)
plt.tight_layout()
plt.savefig("random_allocation.png")
plt.show()
