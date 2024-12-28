<script>
  import { Navbar, NavBrand, Button } from 'flowbite-svelte';
  import { MoonOutline } from 'flowbite-svelte-icons';
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

  <Button color="purple" pill={true} class="!p-2">
    <MoonOutline class="w-5 h-5" />
  </Button>
</Navbar>
