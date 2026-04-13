import csv

x = 4
total = x * 10 * 8
split = x * 8 * 8

rows = []
node2_counter = {}

for num in range(total):
    col1 = num

    if num < split:
        mod64 = num % 64
        a = mod64 // 8
        b = mod64 % 8
        start = a + b
        col2 = list(range(start, 15))
    else:
        rem = num - split
        c = rem // 16
        d = rem % 16
        if 0 <= d <= 7:
            col2 = list(range(8, 16))
        else:
            start = 8 + c
            col2 = list(range(start, 12))

    col3 = []
    if num < split:
        mod8 = num % 8
        mod64 = num % 64
        for i in range(1, 8 - mod8):
            col3.append(num + i)
        for k in range(1, 8 - mod64 // 8):
            col3.append(num + 8 * k)
        col3.append(x * 8 * 8 + mod8 + num // 64 * 16)
    else:
        rem = num - split
        c = rem // 16
        d = rem % 16
        if 0 <= d <= 7:
            start3 = num + 8 - num % 8
            for i in range(8):
                col3.append(start3 + i)
        else:
            current = num + 16
            while current <= total - 1:
                col3.append(current)
                current += 16
            col3.append(x * 10 * 8)

    is_special_case = False
    special_count = 0
    special_port_val = 0
    if num >= split:
        rem = num - split
        c = rem // 16
        d = rem % 16
        if 8 <= d <= 15:
            is_special_case = True
            special_count = x - 1 - c
            special_port_val = 8 + c

    def get_bandwidth(n):
        if n >= split:
            rem = n - split
            d = rem % 16
            if 8 <= d <= 15:
                return "800Gbps"
        return "400Gbps"

    def get_delay(n):
        if n >= split:
            rem = n - split
            d = rem % 16
            if 8 <= d <= 15:
                return "500ns"
        return "100ns"

    for idx, (v2, v3) in enumerate(zip(col2, col3)):

        if is_special_case and idx < special_count:
            portId2 = special_port_val
        else:
            cnt = node2_counter.get(v3, 0)
            portId2 = cnt
            node2_counter[v3] = cnt + 1


        bw = get_bandwidth(num)
        delay = get_delay(num)


        rows.append([col1, v2, v3, portId2, bw, delay])


with open("topology_final.csv", "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["nodeId1", "portId1", "nodeId2", "portId2", "bandwidth", "delay"])
    writer.writerows(rows)
