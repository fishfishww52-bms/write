"""
plot_soc.py — 绘制 SOC 算法结果
生成交互式 HTML，鼠标悬停可看数值
"""
import sys
import pandas as pd
import plotly.graph_objects as go
from plotly.subplots import make_subplots
from pathlib import Path

if len(sys.argv) < 2:
    print(f"用法: python {sys.argv[0]} <soc_result_csv>")
    sys.exit(1)

csv_path = Path(sys.argv[1])
df = pd.read_csv(csv_path)

# 时间轴（100ms/tick）
df["time_s"] = df["tick"] / 10

# 创建交互式子图
fig = make_subplots(
    rows=3, cols=1,
    shared_xaxes=True,
    vertical_spacing=0.06,
    subplot_titles=("SOC", "电压 / 电流", "容量 QNOW"),
)

# SOC
fig.add_trace(
    go.Scatter(x=df["time_s"], y=df["soc_show"], name="SOC 显示",
               line=dict(color="blue"), hovertemplate="t=%{x:.1f}s<br>SOC=%{y}"),
    row=1, col=1)

# 电压 + 电流
fig.add_trace(
    go.Scatter(x=df["time_s"], y=df["voltage"], name="电压 (1/64V)",
               line=dict(color="orange"), hovertemplate="t=%{x:.1f}s<br>V=%{y}"),
    row=2, col=1)
fig.add_trace(
    go.Scatter(x=df["time_s"], y=df["current"], name="电流 (1/64A)",
               line=dict(color="red"), hovertemplate="t=%{x:.1f}s<br>A=%{y}"),
    row=2, col=1)

# 容量
fig.add_trace(
    go.Scatter(x=df["time_s"], y=df["cap_qnow"], name="容量 QNOW",
               line=dict(color="green"), hovertemplate="t=%{x:.1f}s<br>Q=%{y}"),
    row=3, col=1)

fig.update_layout(
    height=700,
    title_text=f"SOC 仿真结果 — {csv_path.stem}",
    hovermode="x unified",  # 同一时刻所有数据一起显示
)

out_html = csv_path.with_suffix(".html")
fig.write_html(out_html)
print(f"OK 已生成: {out_html}")
print(f"   用浏览器打开，鼠标悬停可看数值，支持缩放/平移")
