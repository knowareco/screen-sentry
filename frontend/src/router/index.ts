import { createRouter, createWebHistory } from 'vue-router'
import DeviceHome from '../views/DeviceHome.vue';

const router = createRouter({
    history: createWebHistory(import.meta.env.BASE_URL),
    routes: [
        {
            path: '/',
            name: 'home',
            component: DeviceHome,
        }
    ],
})

export default router
