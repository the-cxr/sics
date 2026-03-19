// SUBSTITUA PELAS SUAS CHAVES REAIS DO SUPABASE
const SUPABASE_URL = 'https://fsqpnjqbbqgvcyuypgpk.supabase.co';
const SUPABASE_KEY = 'eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6ImZzcXBuanFiYnFndmN5dXlwZ3BrIiwicm9sZSI6ImFub24iLCJpYXQiOjE3NzI0NDY2NDEsImV4cCI6MjA4ODAyMjY0MX0.wvWOvOry1MMU888dOb4hyPro751-0YhG7VYgYReUbmE';

let supabaseClient = null;
try {
    supabaseClient = supabase.createClient(SUPABASE_URL, SUPABASE_KEY);
} catch (e) {
    console.error("Erro ao iniciar Supabase. Verifique as chaves.");
}

let map;

function entrar() {
    console.log("Tentando entrar...");
    const login = document.getElementById('login-screen');
    const dashboard = document.getElementById('main-dashboard');

    login.classList.add('hidden');
    dashboard.classList.remove('hidden');

    // Força o mapa a carregar após a transição de tela
    setTimeout(() => {
        initMap();
    }, 100);
}

function initMap() {
    if (map) return; // Evita criar o mapa duas vezes

    // Centralizado em Chapecó (-27.1006, -52.6152)
    map = L.map('map').setView([-27.1006, -52.6152], 14);

    L.tileLayer('https://{s}.basemaps.cartocdn.com/light_all/{z}/{x}/{y}{r}.png', {
        attribution: '© OpenStreetMap'
    }).addTo(map);

    // Pin de exemplo na frente da Arena Condá
    L.marker([-27.1035, -52.6074]).addTo(map)
        .bindPopup('<b>Lixeira Arena Condá</b><br>Status: 25%')
        .openPopup();

    if (supabaseClient) {
        carregarDadosReais();
    }
}

async function carregarDadosReais() {
    // Aqui vai a lógica de buscar do banco que fizemos antes
    console.log("Buscando dados do Supabase...");
}