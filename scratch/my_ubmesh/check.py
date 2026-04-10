# 仅扫描 transport_channel.csv，查找重复行（绝对不修改你的文件）
import os

# 你的文件路径
path = "transport_channel.csv"

# 读取所有行
with open(path, "r", encoding="utf-8") as f:
    lines = [line.strip() for line in f.readlines() if line.strip()]

print(f"📄 文件总行数（不含空行）：{len(lines)}")
if len(lines) == 0:
    print("✅ 文件是空的，没有问题")
    exit()

# 第一行是表头，不算数据
header = lines[0]
data_lines = lines[1:]
print(f"📊 数据行数：{len(data_lines)}")
print(f"📌 表头：{header}")

# 开始检查重复（规则：srcNodeId + dstNodeId + priority 不能重复）
key_map = {}
duplicates = []

for idx, line in enumerate(data_lines):
    row_num = idx + 2  # 真实行号（从1开始算表头）
    parts = line.split(",")
    if len(parts) < 3:
        print(f"❌ 第 {row_num} 行：列数不足！必须至少3列")
        continue

    try:
        src = parts[0].strip()
        dst = parts[1].strip()
        prio = parts[2].strip()
        key = f"{src},{dst},{prio}"

        if key in key_map:
            # 发现重复！
            first_line = key_map[key]
            duplicates.append((row_num, first_line, line))
        else:
            key_map[key] = row_num
    except:
        print(f"❌ 第 {row_num} 行：格式错误！")

# 输出结果
print("\n" + "="*50)
if not duplicates:
    print("✅ transport_channel.csv 没有重复！问题不在这里！")
else:
    print(f"❌ 发现 **{len(duplicates)} 处重复**：")
    for curr, first, content in duplicates:
        print(f"━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
        print(f"🚨 第 {curr} 行 重复了！")
        print(f"🔍 第一次出现：第 {first} 行")
        print(f"📝 重复内容：{content}")

print("\n⚠️ 只要把上面列出的行删掉或修改，仿真就不崩！")