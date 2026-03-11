<script setup>
import { Timer } from '@element-plus/icons-vue'
import { ref, onMounted, onUnmounted } from 'vue'
import { queryAllApi } from '@/api/data'
import { ElMessage } from 'element-plus'

let timer = null
// 必须用 const 定义 ref
const tableData = ref([])

// 时间格式化函数
const formatDateTime = (time) => {
    // 处理空值/无效值
    if (!time) return '';
    // 转换为 Date 对象
    const date = new Date(time);
    // 校验 Date 对象有效性
    if (isNaN(date.getTime())) return '时间格式错误';

    // 年、月、日（补零处理）
    const year = date.getFullYear();
    const month = String(date.getMonth() + 1).padStart(2, '0');
    const day = String(date.getDate()).padStart(2, '0');

    // 时、分、秒（补零处理）
    const hours = String(date.getHours()).padStart(2, '0');
    const minutes = String(date.getMinutes()).padStart(2, '0');
    const seconds = String(date.getSeconds()).padStart(2, '0');

    // 拼接为 YYYY-MM-DD-TT:TT:TT 格式
    return `${year}-${month}-${day}-${hours}:${minutes}:${seconds}`;
};

const handleInquire = async () => {
    try {
        const result = await queryAllApi()

        // 2. 安全赋值：先判断是不是数组
        if (result && Array.isArray(result.data)) {
            tableData.value = result.data.data
            ElMessage.success('查询成功')

        } else if (result && result.data && Array.isArray(result.data.data)) {
            // 兼容：有些接口嵌套了两层 data
            tableData.value = result.data.data
            ElMessage.success('查询成功')
        } else {
            ElMessage.error('数据格式错误，请查看控制台')
            console.error('数据格式错误，期望数组，得到:', result)
        }
    } catch (error) {
        ElMessage.error('请求失败')
        console.error(error)
    }
}

const handleClear = () => {
    tableData.value = [
        {
            writeTime: '',
            temperature: '',
            humidity: ''
        }
    ]
    ElMessage.success('清空成功')
}

// 钩子函数
onMounted(() => {
    handleInquire()
    timer = setInterval(handleInquire, 5000)
})

// 4. 组件销毁前 → 清除定时器
onUnmounted(() => {
    if (timer) {
        clearInterval(timer)
        timer = null
    }
})
</script>

<template>
    <div class="mb-4">
        <el-table :data="tableData" style="width: 100%" border>
            <el-table-column label="Time" width="200">
                <template #default="scope">
                    <div style="display: flex; align-items: center">
                        <el-icon>
                            <Timer />
                        </el-icon>
                        <span style="margin-left: 10px">{{ formatDateTime(scope.row.writeTime) }}</span>
                    </div>
                </template>
            </el-table-column>
            <el-table-column label="Temperature" width="180">
                <template #default="scope">
                    <el-tag type="warning">{{ scope.row.temperature }}&deg;C</el-tag>
                </template>
            </el-table-column>
            <el-table-column label="Humidity" width="180">
                <template #default="scope">
                    <el-tag type="success">{{ scope.row.humidity }} %RH</el-tag>
                </template>
            </el-table-column>
            <el-table-column label="Operations" width="200">
                <template #default>
                    <el-button size="small" @click="handleInquire">Refresh</el-button>
                    <el-button size="small" type="danger" @click="handleClear">Clear</el-button>
                </template>
            </el-table-column>
        </el-table>
    </div>
</template>

<style scoped>
.mb-4 {
    margin: auto 0;
}
</style>