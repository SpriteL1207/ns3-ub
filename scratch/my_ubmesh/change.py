import csv
from collections import defaultdict

# 输入输出文件
INPUT_CSV = "routing_table.csv"
OUTPUT_CSV = "new_8_columns_table_final.csv"

# 新表头（保持不变）
header = ["col1", "col2", "col3", "col4", "col5", "col6", "col7", "col8"]

# 第一步：读取原始数据，并预处理以找到每个node作为col4时的最大col6
filtered_rows = []
temp_col6_data = []  # 临时存储用于计算最大值的数据

with open(INPUT_CSV, "r", encoding="utf-8") as f:
    reader = csv.DictReader(f)
    for row in reader:
        nodeId = int(row["nodeId"])
        dstNodeId = int(row["dstNodeId"])
        dstPortId = row["dstPortId"]
        outPorts = row["outPorts"]
        metrics = row["metrics"]

        if 0 <= nodeId <= 255 and nodeId < dstNodeId:
            # 处理多端口情况
            port_ids = outPorts.split()
            for port_str in port_ids:
                portId1 = int(port_str)
                
                # 存储完整的过滤后的数据
                filtered_rows.append({
                    "nodeId": nodeId,
                    "portId1": portId1,
                    "dstNodeId": dstNodeId,
                    "dstPortId": dstPortId,
                    "metrics": metrics
                })

# 第二步：模拟原始代码中col6的生成过程，以找到每个dstNodeId的最大col6值
# 这是为了确定col3的起始值
col4_counter_for_max = defaultdict(int)
max_col6_for_col4 = defaultdict(int)
for row in filtered_rows:
    dstNodeId = row["dstNodeId"]
    current_col6 = col4_counter_for_max[dstNodeId]
    if current_col6 > max_col6_for_col4[dstNodeId]:
        max_col6_for_col4[dstNodeId] = current_col6
    col4_counter_for_max[dstNodeId] += 1

# 第三步：生成最终数据，严格保留第六列的生成方式
col1_counter = defaultdict(int)  # 用于动态生成col3
col4_counter = defaultdict(int)  # 严格保留用户原始的col6生成方式

final_rows = []
for row in filtered_rows:
    nodeId = row["nodeId"]
    dstNodeId = row["dstNodeId"]
    
    # ✅ 关键修改在这里：动态确定col3的起始值
    if col1_counter[nodeId] == 0:
        # 获取该nodeId作为col4时的最大col6值
        max_col6 = max_col6_for_col4.get(nodeId, -1)
        # 起始值为最大值 + 1
        start_value = max_col6 + 1
        col1_counter[nodeId] = start_value
    
    # 分配当前col3
    col3 = col1_counter[nodeId]
    col1_counter[nodeId] += 1
    
    # ✅ 严格保留第六列的生成方式
    col6 = col4_counter[dstNodeId]
    col4_counter[dstNodeId] += 1
    
    final_rows.append([
        nodeId,
        row["portId1"],  # 使用拆分后的单个端口
        col3,
        dstNodeId,
        row["dstPortId"],
        col6,            # 使用原始方式生成的col6
        7,
        row["metrics"]
    ])

# 写入文件
with open(OUTPUT_CSV, "w", newline="", encoding="utf-8") as f:
    writer = csv.writer(f)
    writer.writerow(header)
    writer.writerows(final_rows)

print(f"✅ 生成完成！文件：{OUTPUT_CSV}")
print(f"📊 总行数：{len(final_rows)} 行")