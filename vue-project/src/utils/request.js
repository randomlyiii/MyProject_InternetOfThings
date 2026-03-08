import axios from 'axios'
import { ElMessage } from 'element-plus'

//创建axios实例对象
const request = axios.create({
    baseURL: '/api',
    timeout: 600000
})

export default request