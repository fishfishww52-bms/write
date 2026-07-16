"""
run_soc_sim.py — 调编译好的 soc_sim.exe 跑 SOC 仿真

用法：
    python run_soc_sim.py <csv_path> [cap] [temp]

文件名自动解析：{容量}_{放电电流}_{组号}.csv  → 自动提取 cap
cellnum 传 0 让算法自动判断（根据初始电压）
temp 默认 25℃
"""
import re
import sys
import subprocess
import pandas as pd
from pathlib import Path

SOC_SIM = Path(__file__).parent / "build" / "soc_sim.exe"


def parse_cap_from_filename(stem: str) -> int:
    """从文件名解析容量：12_10_1 → 12Ah"""
    m = re.match(r"(\d+)_", stem)
    if m:
        return int(m.group(1))
    return 38  # 解析失败默认


def main():
    if len(sys.argv) < 2:
        print(f"用法: python {sys.argv[0]} <csv_path> [cap] [temp]")
        print(f"例:   python {sys.argv[0]} ../data/excel/ms100/12_10_1.csv")
        print(f"      python {sys.argv[0]} ../data/excel/ms100/20_15_1.csv 20 25")
        sys.exit(1)

    csv_path = Path(sys.argv[1])
    if not csv_path.exists():
        print(f"❌ 找不到文件: {csv_path}")
        sys.exit(1)

    if not SOC_SIM.exists():
        print(f"❌ 找不到编译好的算法: {SOC_SIM}")
        print("   请先运行 build.bat 编译")
        sys.exit(1)

    # 参数：cap 优先命令行，否则从文件名解析；cellnum=0 算法自动判断
    cap = int(sys.argv[2]) if len(sys.argv) > 2 else parse_cap_from_filename(csv_path.stem)
    cellnum = 0   # 算法自动判断节数
    temp = int(sys.argv[3]) if len(sys.argv) > 3 else 25

    print(f"文件: {csv_path.name}")
    print(f"配置: cap={cap}Ah  cellnum=auto  temp={temp}℃")

    # 读 CSV
    df = pd.read_csv(csv_path)
    print(f"数据点: {len(df)}")

    # 构造输入：第一行带初始化参数，后续只有 voltage,current
    lines = []
    for i, row in df.iterrows():
        v = int(row["voltage"])
        c = int(row["current"])
        if i == 0:
            init_v = v
            lines.append(f"{v},{c},{cap},{cellnum},{temp},{init_v}")
        else:
            lines.append(f"{v},{c}")

    # 调算法
    print("运行 SOC 算法...")
    result = subprocess.run(
        [str(SOC_SIM)],
        input="\n".join(lines),
        capture_output=True,
        text=True,
        timeout=300,
    )

    if result.returncode != 0:
        print(f"❌ 算法异常退出 (code={result.returncode})")
        print(result.stderr[:500])
        sys.exit(1)

    # 写结果
    out_path = csv_path.parent / f"{csv_path.stem}_soc_result.csv"
    with open(out_path, "w", encoding="utf-8-sig") as f:
        f.write(result.stdout)

    print(f"输出行数: {len(result.stdout.strip().splitlines())-1}")
    print(f"结果保存: {out_path}")

    # 显示摘要
    result_df = pd.read_csv(out_path)
    print(f"\n输出列: {list(result_df.columns)}")
    print(f"\n头5行:")
    print(result_df.head(5).to_string(index=False))
    print(f"\n尾5行:")
    print(result_df.tail(5).to_string(index=False))


if __name__ == "__main__":
    main()
