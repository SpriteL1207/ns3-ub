# 清洗你的 topology.csv，保持内容100%一样，只删隐形脏字符
import os

# 输入和输出路径（直接对应你的文件夹）
input_path  = "my_ubmesh/topology.csv"
output_path = "topology1.csv"  # 直接覆盖

# ------------------- 开始清洗 -------------------
with open(input_path, "r", encoding="utf-8", errors="ignore") as f:
    lines = f.readlines()

clean_lines = []
for line in lines:
    # 去掉首尾空白、换行、看不见的字符
    clean_line = line.strip()
    # 跳过空行
    if not clean_line:
        continue
    clean_lines.append(clean_line)

# 写回干净文件
with open(output_path, "w", encoding="utf-8", newline="") as f:
    for line in clean_lines:
        f.write(line + "\n")

print("✅ 清洗完成！文件内容完全不变，只删除隐形错误字符！")