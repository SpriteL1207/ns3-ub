import random
import csv

random.seed(12345)

# 配置参数
NUM_NODES = 256          # 总节点数 0~255
NUM_SELECTIONS = 128     # 每个src做128次选择
DEST_PER_SELECT = 9      # 每次选9个不同dest
FLOW_SIZE_PER_CNT = 7 * 1024  # 每次计数对应7KB = 7168 Byte

# 1. 读取概率分布文件
prob_file = "dest-pdf-hw-1.txt"
node_probs = []
with open(prob_file, "r") as f:
    for line in f:
        line = line.strip()
        if not line:
            continue
        node_id, prob = line.split()
        node_probs.append((int(node_id), float(prob)))

# 提取node列表和对应的概率列表
nodes = [n for n, p in node_probs]
probs = [p for n, p in node_probs]

# 2. 生成traffic数据
traffic_rows = []
task_id = 0

for src in range(NUM_NODES):
    # 初始化当前src的dest计数器
    cnt = [0] * NUM_NODES
    for _ in range(NUM_SELECTIONS):
        # 每次选9个不同的dest（按概率加权采样）
        selected = random.choices(nodes, weights=probs, k=DEST_PER_SELECT)
        # 去重（如果有重复，补选直到9个不同）
        selected = list(set(selected))
        while len(selected) < DEST_PER_SELECT:
            extra = random.choices(nodes, weights=probs, k=DEST_PER_SELECT - len(selected))
            selected = list(set(selected + extra))
        # 计数+1
        for d in selected:
            cnt[d] += 1
    # 生成流量行（只生成cnt>0的）
    for dest in range(NUM_NODES):
        if cnt[dest] == 0:
            continue
        
        if src == dest:
            continue
                
        data_size = cnt[dest] * FLOW_SIZE_PER_CNT
        traffic_rows.append([
            task_id,
            src,
            dest,
            data_size,
            "URMA_WRITE",
            7,
            "100ns",
            0,
            ""
        ])
        task_id += 1

# 3. 写入CSV
with open("traffic.csv", "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow([
        "taskId", "sourceNode", "destNode", "dataSize(Byte)",
        "opType", "priority", "delay", "phaseId", "dependOnPhases"
    ])
    writer.writerows(traffic_rows)

print(f"生成完成！共 {len(traffic_rows)} 条流量，保存为 traffic.csv")