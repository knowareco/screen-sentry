<script>
  import { Navbar, NavBrand, DarkMode } from 'flowbite-svelte';
</script>

<Navbar>
  <NavBrand href="/">
    <img
      src="https://flowbite.com/docs/images/logo.svg"
      class="mr-3 h-6 sm:h-9"
      alt="Flowbite Logo"
    />
    <span class="self-center whitespace-nowrap text-xl font-semibold dark:text-white">
      Screen Sentry
    </span>
  </NavBrand>

  <DarkMode />
</Navbar>
